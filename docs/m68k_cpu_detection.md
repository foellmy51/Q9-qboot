# Softwarebasierte CPU-Erkennung für die M68K-Familie

Ja, man kann den exakten Typ einer Motorola 68000-basierten CPU (68000, 68010, 68020, 68030, 68040, 68060) rein über Software bestimmen! 

Da spätere CPU-Generationen neue Instruktionen und Kontrollregister eingeführt haben, nutzt man ein bewährtes Verfahren: Man versucht, eine generationenspezifische Instruktion auszuführen. Unterstützt die CPU diese nicht, wird eine **"Illegal Instruction Exception"** (Vektor 4) ausgelöst. Fängt man diese ab, weiß man genau, was die CPU kann und was nicht.

Hier ist der genaue Ablauf und die technische Unterscheidung:

---

## 1. Der Erkennungs-Algorithmus (Schritt-für-Schritt)

### Schritt 1: Unterscheidung MC68000 vs. MC68010+
Der MC68010 führte das **Vector Base Register (VBR)** und den Befehl `MOVEC` (Move Control Register) ein.
*   **Test:** Versuche `MOVEC VBR, D0` auszuführen.
*   **Ergebnis:**
    *   *Illegal Instruction Exception:* Die CPU ist ein **MC68000**.
    *   *Keine Exception:* Die CPU ist ein **MC68010 oder höher**.

### Schritt 2: Unterscheidung MC68010 vs. MC68020+
Der MC68020 führte einen 32-Bit-Kern, Bitfield-Instruktionen und den Cache Control Register (**CACR**) ein.
*   **Test:** Versuche `MOVEC CACR, D0` auszuführen.
*   **Ergebnis:**
    *   *Illegal Instruction Exception:* Die CPU ist ein **MC68010** (da dieser zwar `MOVEC` besitzt, aber kein `CACR`).
    *   *Keine Exception:* Die CPU ist ein **MC68020 oder höher**.

### Schritt 3: Unterscheidung MC68020 vs. MC68030
Der MC68020 besitzt nur einen Befehlscache (Instruction Cache). Der MC68030 besitzt zusätzlich einen Datencache (Data Cache). Dies spiegelt sich im Cache Control Register (**CACR**) wider: Bit 8 (`DCE` - Data Cache Enable) ist beim 68020 schreibgeschützt/immer 0, beim 68030 jedoch beschreibbar.
*   **Test:** Versuche, Bit 8 im `CACR` auf `1` zu setzen und lies das Register wieder aus.
*   **Ergebnis:**
    *   *Bit 8 bleibt 0:* Die CPU ist ein **MC68020**.
    *   *Bit 8 wird 1:* Die CPU ist ein **MC68030** (wie auf unserem **cb030**-Board!).

### Schritt 4: Unterscheidung MC68030 vs. MC68040
Der MC68040 besitzt eine komplett neue MMU-Architektur und neue Caches. Der Befehl `CPUSH` (Cache Push) wurde eingeführt, um Cache-Lines gezielt in den RAM zurückzuschreiben.
*   **Test:** Versuche den Befehl `CPUSH` auszuführen.
*   **Ergebnis:**
    *   *Illegal Instruction Exception:* Die CPU ist ein **MC68030**.
    *   *Keine Exception:* Die CPU ist ein **MC68040 oder höher**.

### Schritt 5: Unterscheidung MC68040 vs. MC68060
Der MC68060 verfügt über das Processor Configuration Register (**PCR**), welches über `MOVEC` angesprochen wird.
*   **Test:** Versuche `MOVEC PCR, D0` auszuführen.
*   **Ergebnis:**
    *   *Illegal Instruction Exception:* Die CPU ist ein **MC68040**.
    *   *Keine Exception:* Die CPU ist ein **MC68060**.

---

## 2. Praktische Umsetzung im Assembler-Bootstrap (Stage 1)

Um diese Abfragen sicher durchzuführen, ohne dass das System abstürzt, biegt man im Interrupt-Vektor 4 (Illegal Instruction) temporär auf einen speziellen Handler ab. 

Hier ist ein typischer Entwurf in 68k-Assembler für Q-Boot Stage 1:

```assembly
|-------------------------------------------------------------------------
| m68k_detect_cpu: Ermittelt den CPU-Typ (0=68000, 1=68010, 2=68020, 3=68030...)
| Gibt den Typ in Register D0 zurück.
|-------------------------------------------------------------------------
m68k_detect_cpu:
    movem.l d1-d2/a0, -(sp)     | Register sichern
    
    | 1. Illegal Instruction Vector (Vektor 4 bei Adresse $10) temporär sichern
    move.l  $10, a0             | Alten Handler sichern
    lea     illegal_handler(pc), a1
    move.l  a1, $10             | Unseren temporären Handler eintragen
    
    | Standard-Annahme: MC68000 (Typ 0)
    moveq   #0, d2              | d2 hält unseren erkannten Typ
    
    | --- Test 1: MC68000 vs MC68010+ ---
    clr.w   trap_triggered      | Flag zurücksetzen
    .word   $4e7b, $0801        | Entspricht: movec vbr, d0 (Erzeugt Illegal Instr. auf 68000)
    tst.w   trap_triggered
    bne     .detection_done     | Wenn Exception ausgelöst wurde -> Es ist ein 68000!
    
    moveq   #1, d2              | Es ist mindestens ein 68010
    
    | --- Test 2: MC68010 vs MC68020+ ---
    clr.w   trap_triggered
    .word   $4e7b, $0002        | Entspricht: movec cacr, d0 (Erzeugt Illegal Instr. auf 68010)
    tst.w   trap_triggered
    bne     .detection_done     | Wenn Exception ausgelöst wurde -> Es ist ein 68010!
    
    moveq   #2, d2              | Es ist mindestens ein 68020
    
    | --- Test 3: MC68020 vs MC68030 ---
    | Wir versuchen, das Data Cache Enable Bit (Bit 8) im CACR zu setzen
    .word   $4e7a, $0002        | movec cacr, d0 (Lese CACR)
    bset    #8, d0              | Versuche Bit 8 (DCE) zu setzen
    .word   $4e7b, $0002        | movec d0, cacr (Schreibe CACR)
    .word   $4e7a, $0002        | movec cacr, d0 (Lese wieder aus)
    btst    #8, d0
    bne     .is_68030           | Wenn Bit 8 gesetzt blieb -> Es ist ein 68030!
    bra     .detection_done     | Andernfalls bleibt es ein 68020.

.is_68030:
    moveq   #3, d2              | Es ist ein 68030!
    
    | (Hier könnten weitere Tests für 040 und 060 folgen...)

.detection_done:
    | Originalen Illegal Instruction Handler wiederherstellen
    move.l  a0, $10
    
    move.l  d2, d0              | Ergebnis in D0 übergeben
    movem.l (sp)+, d1-d2/a0     | Register wiederherstellen
    rts

|-------------------------------------------------------------------------
| Temporärer Exception Handler für ungültige Befehle
|-------------------------------------------------------------------------
illegal_handler:
    move.w  #1, trap_triggered  | Flag setzen, dass Exception auftrat
    
    | Da ein ungültiger Befehl 2 oder 4 Bytes lang sein kann, müssen wir den 
    | PC auf dem Stack anpassen, damit die CPU nach dem 'rte' nicht wieder
    | in denselben ungültigen Befehl läuft und eine Endlosschleife erzeugt.
    | Bei 68010+ liegt der PC im Exception-Frame auf dem Stack bei (sp+2).
    addq.l  #4, 2(sp)           | Überspringe den fehlerhaften movec-Befehl (4 Bytes)
    rte                         | Return from Exception

trap_triggered:
    .word   0
