# ROM-Overlay und Speicher-Remapping (cb030 Spezialfall)

Dieses Dokument beschreibt das hardwarenahe Verfahren des **ROM-Overlays (Memory Remapping)** auf dem **cb030-Board** und wie Q9-Boot Stage 1 dieses sicher und ohne Systemabsturz während des Bootvorgangs deaktiviert.

---

## 1. Das Problem: Warum es ROM-Overlay gibt

Die Motorola 68000er-CPUs (einschließlich des MC68030) sind hartverdrahtet, nach einem Reset ihre ersten beiden 32-Bit-Werte (den Stack Pointer und den Program Counter) von Adresse `0x00000000` und `0x00000004` zu lesen. 
*   **Anforderung:** Beim Einschalten *muss* an Adresse `0x00000000` lesbarer, nicht-flüchtiger Speicher (das ROM) liegen.
*   **Konflikt:** Während des normalen Betriebs müssen das Betriebssystem (Q9-OS) und das BIOS in der Lage sein, Interrupt-Vektoren (die ebenfalls ab `0x00000000` liegen) dynamisch zu verändern. Wäre das ROM dauerhaft dort eingeblendet, wären die Vektoren schreibgeschützt.

---

## 2. Die Lösung: Die ROM-Umschaltung des cb030

Um diesen Konflikt zu lösen, besitzt das cb030-Board eine Hardware-Umschaltung (ROM-Overlay):
1.  **Reset-Zustand (Aktiv):** Nach einem Reset spiegelt der Speicher-Controller das ROM im gesamten Speicherraum bzw. direkt an Adresse `0x00000000` ein.
2.  **Umschaltung (Deaktivieren):** Durch einen gezielten Zugriff (meist ein Schreib- oder Lesezugriff auf eine bestimmte Hardware-Registeradresse, z. B. `0x00F20000` oder einen bestimmten Latch) wird das Overlay abgeschaltet.
    *   **Danach:** Liegt an Adresse `0x00000000` der beschreibbare Hauptspeicher (DRAM/SRAM) und das ROM ist nur noch an seiner permanenten, hohen Adresse (z. B. `0x00F00000` oder `0x00F80000`) eingeblendet.

---

## 3. Der kritische Übergang (Safe Transition Sequence)

Die Deaktivierung des Overlays ist eine der heikelsten Operationen im gesamten Boot-Ablauf. Würde die CPU Code ausführen, der an Adresse `0x00000010` liegt, und in diesem Moment das Overlay umschalten, würde die CPU im nächsten Taktzyklus auf den uninitialisierten RAM an Adresse `0x00000014` zugreifen und sofort abstürzen (Double Bus Fault / Halt).

### Die 4-Schritt-Sicherheitssequenz im Assembler-Bootstrap (Stage 1)

1.  **Bootstrapping im gespiegelten ROM:** Die CPU startet an Adresse `0x00000008` (gespiegeltes ROM).
2.  **Weitsprung in das permanente ROM:** Das Programm führt sofort einen absoluten Weitsprung (`jmp`) in das permanente ROM-Segment an der hohen Adresse aus (z. B. `jmp (ROM_HIGH_BASE + .high_code)`).
    *   **Effekt:** Der Program Counter (PC) der CPU befindet sich nun im hohen Speicherbereich (z. B. bei `0x00F01234`).
3.  **Deaktivierung des Overlays:** Da die CPU nun sicher aus dem hohen ROM-Bereich läuft, greift sie auf die Remap-Registeradresse zu. Das Overlay an Adresse `0x00000000` wird abgeschaltet.
    *   **Effekt:** Der Speicher ab `0x00000000` wird zu RAM. Die CPU läuft im hohen ROM-Bereich ungestört weiter!
4.  **Vektortabelle kopieren:** Die CPU kopiert die Standard-Vektortabelle aus dem ROM in das frisch eingeblendete RAM ab `0x00000000` (oder biegt das Vektor-Basis-Register `VBR` entsprechend um).
5.  **C-Stack initialisieren:** Jetzt ist es sicher, den Stack Pointer auf eine RAM-Adresse zu setzen und das C-BIOS zu starten.

---

## 4. Assembler-Entwurf für `src/boot/cb030_startup.S`

Hier ist der konkrete, hardwarenahe Assembler-Entwurf, der genau diesen Spezialfall des cb030-Boards löst. Er ist modular aufgebaut und kann als spezifisches Bootstrap-Modul eingebunden werden.

```assembly
|-------------------------------------------------------------------------
| File:         src/boot/cb030_startup.S
| Desc:         Stage 1 Assembly Bootstrap fuer cb030 (MC68030 Board)
|               Handhabt das ROM-Overlay Remapping & CPU-Initialisierung
|-------------------------------------------------------------------------

.global _start

| Definitionen für das cb030 Board
ROM_HIGH_BASE     = $00F00000     | Permanente hohe Basisadresse des ROMs
REMAP_TRIGGER_REG = $00F20008     | Registeradresse zur Deaktivierung des ROM-Overlays
RAM_BASE          = $00800000     | Physischer RAM-Anfang
RAM_TOP           = $00FFFFFF     | Ende des 8 MB RAMs (Stack Pointer Ziel)

.section .vectors
| Am Start des ROMs liegen die Reset-Vektoren für das Power-On
    .long   RAM_TOP               | 0x00000000: Initialer Stack Pointer (ISP)
    .long   _start                | 0x00000004: Initialer Program Counter (PC)

.section .text
_start:
    | --- SCHRITT 1: CPU in sicheren Supervisor-Zustand versetzen ---
    ori.w   #$2700, sr            | Interrupts sperren, Supervisor-Modus erzwingen
    
    | --- SCHRITT 2: Weitsprung in das permanente, hohe ROM-Segment ---
    | Dies ist kritisch, damit der Program Counter (PC) nicht mehr im 0x00000000-Bereich liegt!
    jmp     (ROM_HIGH_BASE + .run_in_high_rom)

.run_in_high_rom:
    | --- SCHRITT 3: Deaktivierung des ROM-Overlays ---
    | Wir schreiben einen Dummy-Wert in das Remap-Register.
    | Ab diesem Moment ist der Speicherbereich bei $00000000 echter RAM (noch leer).
    move.b  #$01, (REMAP_TRIGGER_REG)
    nop                           | Pipeline-Synchronisation
    
    | --- SCHRITT 4: Minimale CPU-Initialisierung (MC68030) ---
    | Caches deaktivieren über das Cache Control Register (CACR)
    moveq   #0, d0
    .word   $4e7b, $0002          | movec d0, cacr (Caches aus fuer stabilen RAM-Boot)
    
    | --- SCHRITT 5: Vektortabelle in das RAM kopieren ---
    | Da das RAM bei $00000000 nach dem Remap uninitialisiert ist, kopieren wir 
    | die Reset- und Exceptionvektoren aus dem ROM dorthin, um Exceptions abfangen zu können.
    lea     (ROM_HIGH_BASE), a0   | Quelle im ROM
    moveq   #0, a1                | Ziel im RAM ($00000000)
    move.w  #255, d0              | 256 Vektoren (je 4 Bytes) kopieren
.copy_vectors:
    move.l  (a0)+, (a1)+
    dbra    d0, .copy_vectors

    | --- SCHRITT 6: Vektor-Basis-Register (VBR) setzen ---
    | Wir setzen das VBR auf den RAM-Anfang (VBR = $00000000 oder RAM-Anfang)
    move.l  #0, d0
    .word   $4e7b, $0801          | movec d0, vbr

    | --- SCHRITT 7: Stack aufsetzen und C-BIOS starten ---
    move.l  #RAM_TOP, sp          | Stack Pointer im RAM definieren
    
    | Sprung in das C-Hauptprogramm (Stage 2)
    jsr     main
    
    | Falls main() jemals zurückkehrt: Endlosschleife (Halt)
.halt_system:
    stop    #$2700
    bra     .halt_system
