# Q9 Forge – Roadmap

Die Roadmap beschreibt die geplante Entwicklung von Q9 Forge und seinen Unterprojekten. Zeitangaben werden bewusst nicht festgelegt; die Reihenfolge orientiert sich an Abhängigkeiten und stabilen Zwischenständen.

*English version: [ROADMAP.md](ROADMAP.md)*

v    fertig, eventuell mit kleinen Problemen
-    in Arbeit, wird aktuell aktiv entwickelt
x    geplant, morgen, nächsten Monat oder im nächsten Jahrhundert :-)


## Q9 OS

- Phase 1

    - Kernel-Reverse-Engineering, Disassemblierung          - Dez 2026
    - Kernel-Planung: welche Teilmodule brauchen wir        x 2027
    - Kernel: erste Übersetzung eines vorhandenen Kernels    x 2027
      mit dem Originalcode
    - Kernel neu schreiben in C-Code                        x 2027
    - Systemmodule bauen                                    x 2027/28
    - Manager bauen, untersuchen und planen                 x 2028


## Q9 Flux Emulator

- Phase 1
    - Emulator für Motorola 68K (auf Musashi-Basis)          v
      läuft auf Mac, Linux, Windows
    - Hardware-Simulation für reale/virtuelle Hardware       v
    - Port für OS-9/68k als 68030-CPU                        v
    - virtuelle Netzwerk-Terminal-Simulatoren                v
    - Netzwerk-Unterstützung als Ethernet-Simulation         v
    - Telnet-Unterstützung in OS-9                           v
    - Framebuffer-Unterstützung mit Remote-Support           - Aug 2026

- Phase 2
    - neues virtuelles, schnelles Hardware-Ziel für Q9 Flux  x 2027
    - neues Ziel Hardware mc68000                            x 2027
    - interner Emulator 6809                                 x 2027
    - interne CP/M-68k-Emulation                              x 2027

- Phase 3 -- mehrere Zielarchitekturen (Vorschlag 2026-08-11, Details siehe
  Q9Forge/AI_CONTEXT_de.md "Mehrere Zielarchitekturen" und Q9-Flux/ARBEITSPLAN.md)

    Pro Zielarchitektur zwei Bauarten: Typ A (Board-Emulator, wie heute für
    68K) und Typ B (native Q9-OS-Runtime ohne Hardware-Emulation, Syscalls
    gehen an den echten Host-Kernel).

    - neues Ziel RISC-V32 (Typ A + Typ B)                    x 2028
    - neues Ziel ARM64 (zuerst Typ B -- nativer Q9-Port
      bereits angefangen, Mac-Modell offen; Typ A optional
      später)                                                x 2028
    - neues Ziel x86 32-Bit Protected Mode, flach
      (angelehnt an OS-9000 auf 386+, NICHT Real Mode --
      das ist zwingend 16-Bit; Typ A + Typ B)                x 2028
    - neues Ziel Raspberry (ARM, Typ A)                      x 2029




## Q9 QCC  C-Compiler

- Phase 1
    - EBNF-Parser, rekursiver Top-Down-Parser                v   (erste Version)
    - Codeerzeugung in Zwischencode                          v
    - Zwischencode definiert als Stackmaschine                v
    - Backend für 68k aus Zwischencode                        v

- Phase 2
    - C-Präprozessor                                          x 2027
    - 68k-Assembler für Q9                                    x 2027
    - Linker für Q9-ROF-Format                                x 2028

- Phase 3
    - weitere Frontends, Rust                                 x 2028
    - Interpreter für Zwischencode                            x 2028


## Vinculum
   Hardware mit Motorola 68360,
       Netzwerk,
       32 MByte RAM
       Dual-CF-Laufwerk
       USB-Stick

- Phase 1
       Schaltplan                                             - 2027
       Leiterplatte und Bauteile bestellen                    x 2028
       Erster Prototyp mit laufendem Q9                       x 2028



## Phase 1 – Projektbasis ordnen

- Q9 Forge als übergeordnete Struktur etablieren
- Q9 Flux und Q9 Frame sauber einordnen
- bestehende Projekte und Buildpfade inventarisieren
- gemeinsame Namenskonventionen und Dokumentationsstruktur festlegen
- öffentliche Repository-Struktur vorbereiten

## Phase 2 – Q9 Flux stabilisieren

- Emulator- und Hardware-Simulationsbereiche klar trennen
- Speicher-, I/O- und Geräteabbild dokumentieren
- bestehende Tests und Buildvarianten vereinheitlichen
- reproduzierbare Builds für die unterstützten Plattformen sicherstellen

## Phase 3 – Q9 Frame v1

- separates Video-RAM bei `0xFD000000` anbinden
- MC6845-Registermodell ab `0xFFFFA000` integrieren
- monochromen 1-Bit-Framebuffer implementieren
- Bit- und Byte-Layout festlegen und testen
- Dirty-Tracking bei VRAM-Schreibzugriffen umsetzen

## Phase 4 – Host-Service-Manager

- gemeinsamen nicht-blockierenden Service-Rahmen bereitstellen
- bestehenden Terminal-Service einbinden
- Q9-Frame-TCP-Service ergänzen
- Updatefrequenz, Sendewarteschlangen und Client-Lebenszyklus implementieren

## Phase 5 – Q9 Frame Netzwerk v1

- `HELLO` und `VIDEO_INFO`
- `FRAME_FULL` und `FRAME_UPDATE`
- Vollbildanforderung und Resynchronisierung
- UDP-Discovery im lokalen Netz
- Protokoll- und Verbindungstests

## Phase 6 – Test-Clients

- kleiner Desktop-Referenzclient
- Nearest-Neighbor-Skalierung
- Fenster-, Vollbild- und Minimierungsverhalten
- Test mit verschiedenen Updatefrequenzen und langsamen Clients
- später ESP32-Client mit kleinem Display

## Phase 7 – Öffentliche Veröffentlichung

- README und Quickstart vervollständigen
- Lizenzen und Drittanbieter-Komponenten dokumentieren
- Debugdaten, lokale Pfade und private Konfigurationen entfernen
- CI, Tests und reproduzierbare Builds einrichten
- Versions- und Release-Konvention festlegen
- erster öffentlicher Entwicklungsstand von Q9 Forge

## Phase 8 – Erweiterungen

- Farbmodi und virtuelle DAC-Modelle
- mehrere gleichzeitige Video-Clients
- optionaler UDP-Transport für Video-Updates
- Tastatur- und Mauskanal
- konfigurierbare Pixel- und Speicherformate
- weitergehende MC6845-Timing- und Cursorfunktionen
- spätere Fenster- und Terminalintegration
