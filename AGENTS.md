# Q9-Forge – Projektübersicht

`Q9-Forge` enthält mehrere eigenständige Q9-Projekte. Bevor an einer Aufgabe
gearbeitet wird, muss das passende Projektverzeichnis ausgewählt werden.

## Projekte

| Verzeichnis | Zweck |
|---|---|
| `Q9-QCC/` | Compiler-Sammelrepository und geplante Toolchain-Struktur |
| `Q9-Parsec/` | Parsergenerator `qparsec` |
| `Q9-Run/` | Stack-IR-Interpreter `qrun` |
| `Q9-qr68/` | 68k-Macroassembler `qr68` |
| `Q9-ql68/` | 68k-ROF-Linker `ql68` |
| `Q9-qclib/` | Q9-/OS-9-Laufzeit- und Bibliotheksbestandteile |
| `Q9-x86/` | x86-Compiler- und Toolchain-Prototyp |
| `Q9-Flux-68k/` | 68k-OS-9-Kernel und Emulatorumgebung |
| `Q9-Flux-x86/` | x86-Emulator- und Systemumgebung |
| `Q9-OS/` | OS-9-Kernel und Betriebssystembestandteile |
| `Q9-68k/` | 68k-Projekt-Scaffold und Architekturentwürfe |

## Auswahlregel

1. Aufgabe anhand ihres Gegenstands einem Projekt zuordnen.
2. In das betreffende Projektverzeichnis wechseln.
3. Die dortige `AGENTS.md` lesen, falls vorhanden.
4. Nur die für die Aufgabe erforderlichen Projekte verändern.

Bei Aufgaben zur Compilerstruktur ist zunächst `Q9-QCC/` zu verwenden. Für
Kernelaufgaben sind `Q9-OS/` oder `Q9-Flux-68k/` beziehungsweise
`Q9-Flux-x86/` zuständig. Änderungen an diesen Projekten nicht aus einer
Q9-QCC-Aufgabe heraus vornehmen.

## Arbeitsgrundsatz

Die Verzeichnisse sind eigenständige Git-Repositories. Git-Status, Branch und
laufende Änderungen vor jeder Arbeit prüfen. Keine Änderungen zwischen
Projekten verschieben, ohne die Aufgabe und die Historienübernahme ausdrücklich
zu prüfen.

Für die interne Struktur von `Q9-QCC` gilt zusätzlich:

- [Q9-QCC/AGENTS.md](Q9-QCC/AGENTS.md)
- [Q9-QCC/docs/REPO_STRUCTURE_STANDARD_de.md](Q9-QCC/docs/REPO_STRUCTURE_STANDARD_de.md)
