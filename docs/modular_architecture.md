# Modularität & Baukastensystem (HAL)

Q-Boot ist als modularer Hardware-Baukasten (Hardware Abstraction Layer - HAL) konzipiert. Jedes Hardware-Subsystem wird über eine standardisierte C-Schnittstelle (Struktur mit Funktionszeigern) abstrahiert. Dadurch lässt sich die Ziel-Konfiguration ohne Änderungen am Kern-Bootloader austauschen.

---

## 1. Konsolen-Baukasten (Console Abstraction)

Die Konsole ist das primäre Interface für das Setup-Menü, die interaktive Boot-Shell und Kernel-Logs. 

### A. UART-Konsole (Seriell)
*   **Beschreibung:** Einfacher serieller Treiber. Schreibt direkt in die Sendedatenregister des UART-Chips (z.B. 16550 UART oder ein SoC-eigenes UART).
*   **Vorteil:** Extrem schlank, benötigt minimale RAM-Ressourcen.

### B. Framebuffer-Konsole (Grafisch)
*   **Beschreibung:** Gibt Text direkt in einen Grafik-Videospeicher (VRAM) aus.
*   **Anforderung:** 
    *   **Font Asset:** Ein minimalistisches Font-Bitmap (z.B. 8x8 oder 8x16 Pixel pro Zeichen, oft als Monospace-Array im ROM hinterlegt).
    *   **Software Renderer:** Berechnet bei jedem `putc` die x/y-Bildschirmposition und kopiert die entsprechenden Pixel-Bits aus dem Font-Bitmap in den Framebuffer.
    *   **Scrolling:** Verschiebt bei einem Zeilenumbruch am unteren Bildschirmrand den Framebuffer-Inhalt per `memcpy` um eine Zeichenhöhe nach oben und leert die unterste Zeile.

---

## 2. Massenspeicher & Dateisysteme (Storage Abstraction)

Der Bootloader muss in der Lage sein, Partitionen und einfache Dateisysteme (z.B. FAT16/FAT32 oder ext2) zu lesen, um Kernel-Images zu laden.

*   **Treiber-Module:** SPI-Flash, SD-Karte (über SD-Controller oder bit-banged SPI), IDE/SATA.
*   **Dateisystem-Schicht:** Nutzt die Block-Lese-Funktion des Storage-Treiber-Baukastens, um Metadaten und Dateien zu lesen, ohne zu wissen, auf welchem physikalischen Medium die Daten liegen.

---

## 3. Netzwerk-Boot (Network Abstraction - Zukünftig)

Für die Netzwerk-Unterstützung wird das System um einen Netzwerk-Stack erweitert:

*   **Treiber-Modul:** Ethernet-Controller (z.B. Realtek RTL8139, ASIX oder SoC-eigenes MAC/PHY).
*   **BootP / DHCP Flow:**
    1.  Netzwerktreiber initialisieren.
    2.  Senden eines BootP- oder DHCP-Requests (Broadcast).
    3.  Antwort enthält: Eigene IP-Adresse, Server-IP (TFTP-Server) und den Dateinamen des Boot-Images (z.B. `q9_kernel.bin`).
*   **TFTP-Download:** Herunterladen des Kernel-Images über das triviale Dateiübertragungsprotokoll (TFTP) direkt in das RAM und anschließender Start des OS.

---

## 4. Konfiguration & Zusammenstellung (Build-Time / Runtime)

Es gibt zwei Ansätze, wie der Baukasten konfiguriert werden kann:

1.  **Build-Time (Statisch - Bevorzugt für ROM):** Die Treiber werden über Präprozessor-Direktiven (z.B. `CONFIG_CONSOLE_UART` oder `CONFIG_CONSOLE_FRAMEBUFFER` in einer `config.h`) fest eincompiliert.
2.  **Runtime-Registry (Dynamisch):** Treiber registrieren sich zur Laufzeit in einer Treiber-Tabelle. Das System wählt anhand von Setup-Variablen aus dem I2C-Memory den aktiven Treiber.
