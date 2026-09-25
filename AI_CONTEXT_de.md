# Q9 Forge – gemeinsamer KI-Projektkontext

Diese Datei enthält den projektübergreifenden Grundkontext. Er gilt auch dann,
wenn gerade nur an einem einzelnen Teilprojekt gearbeitet wird.

## Verbindliche Namen

- **Q9 Forge** ist das übergeordnete Projekt für alle Q9-Komponenten,
  Werkzeuge und Teilprojekte.
- **Q9 Flux** ist ausschließlich der Emulator. Er emuliert die Q9-Hardware
  und führt Q9-OS aus. Der bisherige technische Binärname `q9.exe` wird bei
  einer späteren, geplanten Umbenennung angepasst.
- **Q9** bzw. **Q9-OS** bezeichnet das Betriebssystem.
- **QCC** ist der C-Compiler und eines der zentralen Endergebnisse des
  Projekts.
- **Parsec** ist der Parser- und Grammatikgenerator. Parsec gehört grob zur
  QCC-Toolchain und unterstützt deren Frontend- und Parser-Erzeugung.
- **QCC-Backends** enthalten die Zielcode-Erzeugung, zum Beispiel für 68k und
  Q9-OS. Weitere Zielplattformen können hinzukommen.

## Einordnung

```text
Q9 Forge
├── Q9 Flux             Emulator
├── Q9 / Q9-OS          Betriebssystem
└── QCC-Toolchain
    ├── Parsec          Parser-/Grammatikgenerator
    ├── QCC             C-Compiler
    └── Backends        z. B. 68k für Q9-OS
```

**Q9 Flux**, **Q9-OS** und **QCC** sind damit eigenständige Bestandteile von
**Q9 Forge**. Parsec ist kein weiterer Emulator, sondern ein Werkzeugbereich
der QCC-Toolchain.

Die Schreibweise **Q9 Flux** ist verbindlich; **Q9 Flex** ist für den Emulator
nicht zu verwenden.

## Mehrere Zielarchitekturen (💡 Vorschlag, 2026-08-11, noch nicht final besprochen)

Bisher unterstützt Q9 Forge ausschließlich die Motorola-68K-Familie (Q9 Flux
als Board-Emulator, Q9-OS als Gast-Betriebssystem darin). Andreas möchte
das auf weitere Zielarchitekturen ausweiten: **RISC-V32**, **ARM64** (der
bereits angefangene native Q9-Port, konkretes Mac-Modell noch offen) und
**x86 32-Bit Protected Mode** (flach, angelehnt an OS-9000 auf 386+ — echtes
Real Mode ist zwingend 16-Bit und daher keine Kandidatenbasis für einen
32-Bit-Kernel).

Pro Zielarchitektur gibt es zwei grundverschiedene Bauarten, die beide unter
Q9 Forge gehören, aber unterschiedliche Projekte/Verzeichnisse sind:

- **Typ A – Board-Emulator**: vollständige Hardware-Emulation (CPU + Board-
  Peripherie), läuft als Host-Anwendung auf Linux/Windows/Mac, führt
  unveränderten Gast-Code aus (z. B. echtes OS-9/68K unter Q9 Flux). Das ist
  die heutige Bauart von **Q9 Flux**, bisher nur für Motorola 68K.
- **Typ B – native Runtime**: Q9-OS selbst, für die Zielarchitektur nativ
  übersetzt, läuft direkt auf echter Hardware bzw. echtem Host-Kernel. Keine
  CPU-/Hardware-Emulation; nur eine dünne HAL übersetzt Q9-OS-Systemaufrufe in
  native Host-Aufrufe (POSIX/Win32/etc.). Entspricht dem archivierten
  Mini-Kernel-Konzept (`Q9RESUME-Kernel`, HAL native/posix/wasm).

Namen für Typ A/Typ B sowie die Verzeichnis-/Repo-Struktur pro Zielarchitektur
sind mit Andreas noch final festzulegen — siehe `ROADMAP.md` und
`Q9-Flux/ARBEITSPLAN.md` für den aktuellen Planungsstand.
