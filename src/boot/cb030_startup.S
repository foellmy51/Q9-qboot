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
    | Wir setzen das VBR auf den RAM-Anfang (VBR = $00000000)
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
