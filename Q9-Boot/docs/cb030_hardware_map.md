# Hardware-Integration: cb030 (MC68030 Board)

Dieses Dokument beschreibt die Hardware-Belegung und die Low-Level-Integration von **Q9-Boot** auf dem **cb030-System** (Motorola 68030 Computer-Board), welches im Emulator nachgebildet ist.

---

## 1. Systemspezifikationen & Speicherbelegung (Memory Map)

Der MC68030 verfügt über einen 32-Bit Adressraum (4 GB). Für das cb030-System gilt folgende typische Speicheraufteilung:

| Adressbereich | Größe | Typ | Beschreibung / Belegung |
| :--- | :--- | :--- | :--- |
| `0x00000000 - 0x0007FFFF` | 512 KB | **ROM / Flash** | Enthält Q9-Boot Stage 1 (Reset-Vektoren) und Stage 2 (BIOS/Shell) |
| `0x00800000 - 0x00FFFFFF` | 8 MB | **SRAM / DRAM** | Hauptspeicher (System-RAM) |
| `0x00F00000 - 0x00F0000F` | 16 Bytes | **I/O (UART)** | Register des seriellen Bausteins (z.B. MC68681 DUART oder 16550) |
| `0x00F10000 - 0x00F100FF` | 256 Bytes | **I/O (Storage)** | IDE-Controller oder SD-Karten-Interface (Block Storage) |
| `0x00F20000 - 0x00F2000F` | 16 Bytes | **I/O (I2C NVRAM)**| SPI/I2C-Controller für das Setup-EEPROM (Konfigurationsdaten) |

---

## 2. Boot-Ablauf auf dem MC68030 (Stage 1)

Nach dem Einschalten (Power-On) oder Reset führt die MC68030-CPU folgende Schritte aus:

1.  **Vektoren laden:** Die CPU liest automatisch die ersten beiden 32-Bit-Werte aus dem ROM (Adresse `0x00000000`):
    *   `0x00000000`: Initialer **Stack Pointer (ISP)** -> Zeigt auf das Ende des RAMs (z.B. `0x01000000`).
    *   `0x00000004`: Initialer **Program Counter (PC)** -> Einstiegspunkt von Q9-Boot Stage 1 (z.B. `0x00000400`).
2.  **CPU-Initialisierung (Assembly):**
    *   Deaktivieren der Caches (Instruction und Data Cache über das **CACR** - Cache Control Register).
    *   Vektor-Basis-Register (**VBR**) initialisieren. Beim 68030 zeigt das VBR standardmäßig auf `0x00000000` (ROM). Um Interrupt-Vektoren im RAM dynamisch verändern zu können, kopiert Stage 1 die Vektortabelle in das RAM (z.B. ab `0x00800000`) und biegt das VBR dorthin um.
3.  **RAM-Test:** Ein schneller Test des DRAMs (`0x00800000` aufwärts), um Schreib-/Lesefähigkeit vor dem Laden von C-Code zu prüfen (POST).
4.  **C-Stack aufsetzen:** Setzen des User- und Supervisor-Stack-Pointers (USP/SSP) im RAM.
5.  **Sprung in Stage 2:** Einstieg in das in C geschriebene BIOS-Hauptprogramm (`src/boot/main.c`).

---

## 3. Einbindung der BIOS-Vektortabelle

Auf dem cb030-System wird die `bios_vector_table` an einer bekannten Adresse im ROM (z.B. fest gelinkt bei `0x00001000`) platziert. 

Jedes Betriebssystem (wie Q9-OS) kann diese Tabelle beim Start lokalisieren und erhält sofortigen Zugriff auf die standardisierten Treiber für:
*   `/term0` (UART-Konsole)
*   `/h0` (SD-Karte / IDE-Platte)

### Beispiel: Aufruf aus einem Q9-OS Treiber (in C)

```c
#define BIOS_VECTOR_ADDR 0x00001000
const struct q9_bios_vectors *bios = (const struct q9_bios_vectors *)BIOS_VECTOR_ADDR;

void write_to_terminal(const char *str) {
    while (*str) {
        bios->console_putc(*str++);
    }
}
```

---

## 4. Hardware-Setup im I2C EEPROM

Der cb030 verfügt über ein kleines I2C-EEPROM (z.B. 24C16) an Adresse `0x00F20000`. 
*   **Zweck:** Permanenter Speicher für die Setup-Variablen (`bootdelay`, `bootorder`, `baudrate`).
*   **Schutz:** Die Variablen werden mit einer einfachen CRC16-Prüfsumme versehen. Schlägt die Prüfung beim Einschalten fehl, verwendet Q9-Boot die Standardwerte im Flash-ROM und warnt den Benutzer in der Shell.
