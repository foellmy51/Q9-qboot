# Softwarebasierte CPU-Erkennung für die M68K-Familie (Erweitert)

Dieses Dokument beschreibt das vollständige, hochentwickelte Verfahren zur dynamischen Software-Erkennung aller wichtigen Prozessoren der Motorola 68000 (m68k) Familie, einschließlich der Mikrocontroller-Linien (**CPU32/683xx**), der Spezial-Derivate (**Signetics 68070**) sowie der Low-Cost- und Embedded-Varianten (**EC, LC, HC, RC**).

---

## 1. Übersicht der Prozessoren & Unterscheidungsmerkmale

| CPU / Core | VBR | CACR | FPU (on-chip) | MMU (on-chip) | Besonderheiten |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **MC68000** | Nein | Nein | Nein | Nein | Ur-CPU (NMOS) |
| **MC68HC000**| Nein | Nein | Nein | Nein | High-Speed CMOS, funktionsgleich zu 68000 |
| **SCC68070** | Nein | Nein | Nein | Einfach | Signetics Core, integrierte Timer/I2C/UART ab `0x80001000` |
| **MC68010** | Ja | Nein | Nein | Nein | Erste Virtualisierungs-CPU (Loop-Mode, VBR) |
| **CPU32 (683xx)**| Ja | Nein | Nein | Nein | MC68332/340/360, unterstützt scaled-indexing, kein `CACR` |
| **MC68020** | Ja | Ja | Extern | Extern | 32-Bit-Kern, Instruction-Cache |
| **MC68EC020**| Ja | Ja | Extern | Extern | Reduzierter Adressbus (24-Bit), kein MMU-Support |
| **MC68030** | Ja | Ja | Extern | Ja | Separater Instruction- & Data-Cache, integrierte MMU |
| **MC68EC030**| Ja | Ja | Extern | Nein | Keine MMU (MMU-Befehle triggern Exception) |
| **MC68040** | Ja | Ja | Ja | Ja | Harvard-Architektur, Dual-Caches, FPU & MMU integriert |
| **MC68LC040**| Ja | Ja | Nein | Ja | Low-Cost: Keine FPU (F-Line Exception bei Mathe-Befehlen) |
| **MC68EC040**| Ja | Ja | Nein | Nein | Embedded: Keine FPU, keine MMU |
| **MC68060** | Ja | Ja | Ja | Ja | Superskalar, superschnelle FPU & MMU |
| **MC68LC060**| Ja | Ja | Nein | Ja | Keine FPU |
| **MC68EC060**| Ja | Ja | Nein | Nein | Keine FPU, keine MMU |

---

## 2. Der erweiterte Erkennungs-Algorithmus

Der Algorithmus läuft kaskadierend von der Ur-CPU bis hin zu den High-End-Prozessoren. Er stützt sich auf das gezielte Abfangen von **"Illegal Instruction Exceptions" (Vektor 4)** sowie **"Line 1111 (F-Line) Emulator Exceptions" (Vektor 11)**.

### Stufe 1: Ur-Core vs. Virtualisierungs-Core (VBR-Test)
*   **Test:** Versuche `MOVEC VBR, D0` auszuführen.
*   **Ergebnis:**
    *   *Illegal Instruction:* CPU gehört zur **68000/68070-Klasse** (weiter zu Stufe 1B).
    *   *Keine Exception:* CPU gehört zur **68010+ Klasse** (weiter zu Stufe 2).

#### Stufe 1B: MC68000 vs. Signetics SCC68070
*   **Test:** Der SCC68070 besitzt einen internen I/O-Block für serielle Ports, DMA und Timer, der standardmäßig fest bei `0x80001000` bis `0x80002000` liegt. Wir prüfen, ob an dieser Adresse physisch lesbare Register existieren (Bus-Error fangen!) und ob sich z.B. das Timer-Register verändern lässt.
*   **Ergebnis:**
    *   *Bus-Error oder keine Reaktion:* **MC68000** (bzw. **68HC000**).
    *   *Erfolgreicher Registerzugriff:* **SCC68070** (Philips/Signetics).

---

### Stufe 2: MC68010 vs. CPU32 vs. MC68020+
*   **Test:** Versuche `MOVEC CACR, D0` auszuführen.
*   **Ergebnis:**
    *   *Keine Exception:* Die CPU ist ein **MC68020/030/040/060** (weiter zu Stufe 3).
    *   *Illegal Instruction:* Die CPU ist ein **MC68010** oder eine **CPU32 (MC683xx)**.

#### Stufe 2B: MC68010 vs. CPU32 (MC683xx)
*   **Test:** Die CPU32-Klasse (e.g. 68332, 68340) besitzt zwar keinen Cache (und somit kein `CACR`), unterstützt aber im Gegensatz zum 68010 erweiterte 68020-Adressierungsmodi wie das **scaled-indexing** (z. B. `(A0, D0.L*4)`).
*   **Test-Assembler:** Versuche eine skalierte Adressierung auszuführen (z. B. `move.b (a0, d0.l*4), d1`).
*   **Ergebnis:**
    *   *Illegal Instruction:* Die CPU ist ein **MC68010**.
    *   *Keine Exception:* Die CPU ist ein **MC683xx (CPU32-Core)**.

---

### Stufe 3: MC68020 vs. MC68030
*   **Test:** Das Cache Control Register (`CACR`) auslesen, Bit 8 (`DCE` - Data Cache Enable) setzen, zurückschreiben und erneut auslesen.
*   **Ergebnis:**
    *   *Bit 8 bleibt 0:* Die CPU ist ein **MC68020** (weiter zu Stufe 3B).
    *   *Bit 8 wird 1:* Die CPU ist ein **MC68030** (weiter zu Stufe 3C).

#### Stufe 3B: MC68020 Bus-Breite (MC68EC020)
*   Der MC68EC020 ist eine Embedded-Variante mit einem auf 24-Bit beschnittenen Adressbus (16 MB maximal).
*   **Test:** Versuche, auf eine Speicheradresse über 16 MB zuzugreifen (z. B. `0x01000000`), schreibe einen Wert hinein, und prüfe auf ein "Mirroring" (Spiegelung) bei Adresse `0x00000000`.
*   **Ergebnis:**
    *   *Mirroring vorhanden:* **MC68EC020**.
    *   *Kein Mirroring:* Voller **MC68020**.

#### Stufe 3C: MC68030 vs. MC68EC030 (MMU-Test)
*   Der MC68EC030 besitzt im Gegensatz zum vollen 68030 **keine MMU**.
*   **Test:** Führe einen MMU-Befehl aus (z. B. `PMOVE` um das MMU-Translation-Control-Register `TC` zu lesen).
*   **Ergebnis:**
    *   *Illegal/F-Line Instruction:* **MC68EC030** (Embedded-Version ohne MMU).
    *   *Keine Exception:* Voller **MC68030** (mit MMU).

---

### Stufe 4: MC68030 vs. MC68040
*   **Test:** Versuche den 68040-spezifischen Cache-Befehl `CPUSH` auszuführen.
*   **Ergebnis:**
    *   *Illegal Instruction:* Die CPU ist ein **MC68030**.
    *   *Keine Exception:* Die CPU ist ein **MC68040-Derivat** (weiter zu Stufe 4B).

#### Stufe 4B: Vollwertiger MC68040 vs. LC040 vs. EC040
Die LC- und EC-Varianten des 68040 sparen FPU und MMU ein:
1.  **FPU-Test (F-Line Exception):**
    *   Versuche ein mathematisches FPU-Kommando auszuführen (z. B. `FNOP` oder `FMOVE FP0, D0`).
    *   *Triggert Line 1111 (F-Line) Exception (Vektor 11):* **Keine FPU** vorhanden (LC040 oder EC040).
    *   *Keine Exception:* **FPU vorhanden** (Vollwertiger MC68040 oder RC040).
2.  **MMU-Test (auf den FPU-losen Varianten):**
    *   Führe den 68040 MMU-Befehl `PTEST` aus.
    *   *Triggert Exception:* **Keine MMU** vorhanden -> **MC68EC040**.
    *   *Keine Exception:* **MMU vorhanden** -> **MC68LC040**.

---

### Stufe 5: MC68040 vs. MC68060
*   **Test:** Versuche das 68060-spezifische Kontrollregister `PCR` (Processor Configuration Register) mittels `MOVEC PCR, D0` auszulesen.
*   **Ergebnis:**
    *   *Illegal Instruction:* CPU ist ein **MC68040**-Derivat.
    *   *Keine Exception:* CPU ist ein **MC68060-Derivat** (weiter zu Stufe 5B).

#### Stufe 5B: Vollwertiger MC68060 vs. LC060 vs. EC060
Analog zum 68040:
1.  **FPU-Test:** Führe `FNOP` aus.
    *   *F-Line Exception:* **Keine FPU** (LC060 oder EC060).
    *   *Keine Exception:* **FPU vorhanden** (Vollwertiger MC68060).
2.  **MMU-Test (auf den FPU-losen Varianten):**
    *   Führe `PTEST` aus.
    *   *Exception:* **Keine MMU** -> **MC68EC060**.
    *   *Keine Exception:* **MMU vorhanden** -> **MC68LC060**.

---

## 3. Zusammenfassung des Probe-Ablaufs im BIOS

Mit dieser kaskadierenden Logik kann das `Q-Boot` BIOS beim Start ein extrem präzises Profil der CPU und des Boards erstellen. Die Ergebnisse können im System-Log ausgegeben werden:

```text
==================================================
         Q-Boot Bootloader & BIOS v1.0.0
==================================================
POST: Pruefe CPU Register... OK
POST: Pruefe RAM-Segmente... OK

CPU-Probing:
- VBR-Support  : JA
- CACR-Support : JA
- Data Cache   : JA (Bit 8 DCE beschreibbar)
- Cache-Push   : NEIN (CPUSH illegal)
- MMU-Support  : JA (PMOVE TC erfolgreich)
- FPU-Support  : NEIN (FNOP triggert F-Line)

-> Identifizierter Prozessor: Motorola MC68EC030 (Embedded, No MMU, No FPU)
==================================================
```
