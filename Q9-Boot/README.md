# Q9-Boot

**Q9-Boot** ist das dedizierte Bootloader-, BIOS- und Startsystem für die **Q9-Architektur**. Es stellt die Brücke zwischen der nackten Hardware nach dem Einschalten (Power-On) und dem Laden des vollständigen Betriebssystems dar.

---

## Hauptfunktionen & Architektur

### 1. True Bootstrap (Stage 1)
*   **Startort:** Wird direkt aus dem ROM oder Flash-Speicher ausgeführt.
*   **Hardware-Initialisierung:**
    *   Setzen der CPU-Register in einen definierten Zustand.
    *   RAM-Initialisierung und -Training (falls notwendig).
    *   Vektortabelle (Interrupt Vector Table) initialisieren und im RAM/ROM platzieren.
    *   POST (Power-On Self-Test) zur Prüfung lebenswichtiger Komponenten (CPU, RAM).

### 2. BIOS & System Services (Stage 2)
*   **Minimal Hardware Initialisierung:**
    *   UART/Terminal für serielle Kommunikation (CLI/Logging).
    *   Einfacher Display-Treiber (falls anwendbar).
    *   Massenspeichertreiber (SD-Karte, Flash, IDE/SATA o.ä.) zur Dateisystem-Unterstützung.
*   **BIOS API Vector:** Bereitstellung von Software-Interrupts oder API-Vektoren für Betriebssysteme zur Nutzung von Standard-I/O-Routinen (z. B. Zeichen ausgeben, Sektor lesen).

### 3. Setup & Hardware-Konfiguration
*   **NVRAM / I2C EEPROM:**
    *   Auslesen und Schreiben von Konfigurationsdaten über ein I2C-EEPROM oder eine dedizierte Partition im Massenspeicher.
    *   **Inhalt des Setups:**
        *   Bootreihenfolge (z. B. SD-Karte -> Flash -> UART-Load).
        *   Hardware-Kalibrierungsdaten.
        *   Standard-Boot-Verzögerung (Delay).
        *   Terminal-Baudrate.
*   **Interaktives Setup-Menü:** Zugriff über die Boot-Shell zur manuellen Konfiguration der Systemvariablen.

### 4. Kalibrierung & Qumips-Benchmark
*   **Kalibrierungsroutine:** Ermittlung von CPU-Taktraten und Anpassung von hardwareabhängigen Delays.
*   **Qumips-Benchmark:** Bestimmung der relativen CPU-Performance (Qumips) direkt während des Bootvorgangs zur Validierung der Systemgeschwindigkeit und Stabilität.

### 5. Bootloader-Shell / CLI
*   **Interaktive Steuerung:** Ein U-Boot-ähnliches Terminal, das bei Tastendruck (z.B. ESC oder Leertaste) während des Boot-Coundowns gestoppt wird.
*   **Befehle (geplant):**
    *   `help` - Befehlsübersicht.
    *   `md` / `mw` - Memory Dump / Memory Write (RAM-Inspektion).
    *   `load` - Laden von Binaries über UART (z. B. XMODEM/YMODEM).
    *   `printenv` / `setenv` / `saveenv` - Verwalten der Setup-Variablen im I2C-Memory.
    *   `boot` - OS manuell starten.

### 6. OS-Identifikation & Bootstrapping (Stage 3)
*   **Massenspeicher scannen:** Partitionstabelle (MBR/GPT) oder Dateisystem (FAT/ext2) lesen.
*   **Integritätsprüfung:** Validierung des OS-Kernels mittels CRC32- oder SHA-Prüfsumme.
*   **Vorbereitung:** Übergeben von Boot-Parametern (A-Tag, Device-Tree oder Kernel-Argumente) im RAM.
*   **Start:** Übergabe der Kontrolle an das Betriebssystem.

---

## Projektstruktur

```text
Q9-Boot/
├── docs/                 # Spezifikationen, Registerbelegungen, I2C-Memory Maps
├── src/
│   ├── boot/             # Stage 1 Bootstrap (Assembly & Low-Level Initialisierung)
│   ├── bios/             # BIOS-Routinen, Interrupt-Vektoren und I/O-Treiber
│   ├── setup/            # Einstellungen, I2C-EEPROM-Treiber, Setup-Menü
│   ├── cli/              # Interaktive Bootloader-Shell (U-Boot-like CLI)
│   ├── lib/              # CRC32, Qumips-Benchmark, String-Hilfsfunktionen
│   └── include/          # Gemeinsame Header-Dateien (.h)
└── README.md             # Dieses Dokument
```
