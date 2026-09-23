# Speichersuche und Speicherprüfung (Memory Probe & POST)

Dieses Dokument beschreibt das hardwarenahe Verfahren zur dynamischen Erkennung der RAM-Größe (Memory Probe) sowie die anschließende Integritätsprüfung (POST - Power-On Self-Test) auf dem **cb030-Board** und anderen Q9-Hardwareplattformen.

---

## 1. Dynamische Speichererkennung (Memory Probing)

Auf nackter Hardware (Bare-Metal) kann der verfügbare RAM-Ausbau variieren (z. B. 4 MB, 8 MB oder 16 MB SRAM/DRAM). Um den Speicher dynamisch zu bestimmen, nutzen wir ein kaskadierendes Prüfverfahren in 1-Megabyte-Schritten.

### A. Schutz vor Systemabstürzen (Bus-Error Handling)
Greift die CPU auf eine Speicheradresse zu, die physikalisch nicht existiert, löst der Bus-Controller (oder DRAM-Controller) des cb030 einen **Bus Error (BERR)** aus. Die CPU reagiert darauf mit einer **Bus Error Exception (Vektor 2 bei Adresse `$08`)**.
*   **Verfahren:** Vor der Prüfung biegen wir Vektor 2 temporär auf einen speziellen BIOS-Handler ab. Tritt ein Bus-Error auf, setzt der Handler ein Flag und springt sicher zurück.

### B. Erkennung von Adress-Spiegelung (Mirroring / Wrap-around)
Einige Speicher-Controller ignorieren höhere Adressleitungen, wenn der RAM-Ausbau kleiner ist als das theoretische Maximum. Dadurch "spiegelt" sich der Speicher (Wrap-around): Ein Schreibzugriff auf `0x00800000 + 8 MB` überschreibt physikalisch die Adresse `0x00800000`.
*   **Verfahren:** Bei jedem Schritt schreiben wir ein Testmuster an die neue Adresse und prüfen, ob sich der Wert an der Basisadresse `0x00800000` unerwartet verändert hat.

---

## 2. Der Erkennungs-Algorithmus (C-Pseudocode)

Dieser Code wird sehr früh im BIOS-Ablauf ausgeführt (direkt nach der Konsoleninitialisierung), um den Heap und Stack sicher im RAM zu verankern.

```c
#define RAM_START          0x00800000
#define PROBE_STEP         (1024 * 1024)   /* 1 MB Schritte */
#define MAX_RAM_LIMIT      (32 * 1024 * 1024) /* 32 MB max */

volatile int bus_error_triggered = 0;

void bus_error_handler(void); /* In Assembler geschrieben, setzt bus_error_triggered = 1 */

uint32_t probe_ram_size(void) {
    uint32_t current_size = 0;
    uint32_t base_addr = RAM_START;
    
    /* 1. Bus-Error-Vektor (Vektor 2, Adresse 0x08) temporär sichern und verbiegen */
    void (*old_handler)(void) = *(void (**)(void))0x08;
    *(void (**)(void))0x08 = bus_error_handler;
    
    /* Sicherstellen, dass die Basisadresse schreibbar ist */
    volatile uint32_t *base_ptr = (volatile uint32_t *)base_addr;
    uint32_t original_base_val = *base_ptr;
    
    for (current_size = PROBE_STEP; current_size < MAX_RAM_LIMIT; current_size += PROBE_STEP) {
        uint32_t target_addr = base_addr + current_size;
        volatile uint32_t *target_ptr = (volatile uint32_t *)target_addr;
        
        bus_error_triggered = 0;
        
        /* 2. Test-Muster schreiben */
        uint32_t test_val = 0x55AA55AA ^ target_addr;
        *target_ptr = test_val;
        
        /* Kam es zu einem Bus-Error? */
        if (bus_error_triggered) {
            break; /* Physikalische Speichergrenze erreicht! */
        }
        
        /* 3. Test-Muster zurücklesen */
        uint32_t read_val = *target_ptr;
        if (read_val != test_val) {
            break; /* Speicher liest sich nicht korrekt (z.B. Open Bus) */
        }
        
        /* 4. Spiegelungs-Prüfung */
        *base_ptr = 0x12345678; /* Basis überschreiben */
        if (*target_ptr == 0x12345678) {
            /* Die neue Adresse spiegelt sich auf die Basisadresse! */
            break; 
        }
    }
    
    /* Original-Wert der Basisadresse wiederherstellen */
    *base_ptr = original_base_val;
    
    /* Originalen Bus-Error-Handler wiederherstellen */
    *(void (**)(void))0x08 = old_handler;
    
    return current_size;
}
```

---

## 3. Speicherprüfung (POST - Power-On Self-Test)

Nachdem die genaue RAM-Größe ermittelt wurde, führt das BIOS eine schnelle, strukturierte Speicherprüfung durch, um defekte RAM-Bausteine oder kalte Lötstellen an den Adress-/Datenleitungen auszuschließen.

### A. Datenbus-Test (Walking 1s und 0s)
Prüft auf Kurzschlüsse zwischen den Datenleitungen oder Unterbrechungen.
*   **Methode:** Es wird nacheinander eine `1` durch alle 32 Bit-Positionen geschoben (`0x00000001`, `0x00000002`, `0x00000004` ...) an eine feste Adresse geschrieben und verifiziert. Danach das Gleiche mit Nullen (`0xFFFFFFFE` ...).

### B. Adressleitungs-Test
Prüft auf Kurzschlüsse zwischen den Adressleitungen (Überlappung).
*   **Methode:** Es werden unterschiedliche Werte an Adressen geschrieben, die Zweierpotenz-Offsets entsprechen (z. B. `base + 1`, `base + 2`, `base + 4`, `base + 8` ...). Anschließend wird geprüft, ob sich die Werte gegenseitig überschrieben haben.

### C. Schneller BIOS-RAM-Check (Muster-Füllung)
*   Das BIOS beschreibt den gesamten RAM blockweise mit einem alternierenden Muster (`0xAA55AA55` und `0x55AA55AA`).
*   Anschließend wird der RAM gelesen und verifiziert.
*   **Optimierung für schnellen Boot:** Um den Startvorgang nicht unbarmherzig zu verzögern, kann das BIOS im Setup eine Option `fastboot=1` anbieten, welche den vollständigen RAM-Test überspringt und nur die Grenzen prüft (16 KB Boundaries).

---

## 4. Einbindung in den modularen Q-Boot Boot-Ablauf

Das cb030-BIOS führt die Module in einer präzisen, optimierten Reihenfolge aus, damit der Entwickler maximal früh Feedback über die Konsole erhält:

```text
1. [Stage 1 Assembly] -> Minimale CPU-Init (VBR, Stacks setzen)
2. [Stage 2 C-BIOS]   -> Konsole initialisieren (Sehr frueh! Ab hier Ausgaben moeglich)
                         "Initializing Console on /term0... OK"
3. [Stage 2 C-BIOS]   -> CPU-Erkennung (Prober ausführen)
                         "CPU Detected: MC68030"
4. [Stage 2 C-BIOS]   -> Speichererkennung & POST (Bus-Error & Spiegelungs-Prüfung)
                         "Probing Memory: 8 MB RAM detected at 0x00800000"
                         "Memory POST: Walking 1s... OK, Pattern Fill... OK"
5. [Stage 2 C-BIOS]   -> Qumips Kalibrierungs-Benchmark
                         "Running Calibration... 12.50 Qumips"
6. [Stage 2 C-BIOS]   -> BIOS-Vektoren & Deskriptoren abschließen
                         "BIOS Vector Table initialized at 0x00001000"
7. [Stage 2 C-BIOS]   -> Autostart-Countdown & Shell-Abzweig
```
