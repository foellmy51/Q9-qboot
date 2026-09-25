# Q9 Forge – shared AI project context

This file contains the cross-project baseline context. It applies even when
work is currently focused on a single subproject only.

*German version: [AI_CONTEXT_de.md](AI_CONTEXT_de.md)*

## Binding names

- **Q9 Forge** is the umbrella project for all Q9 components, tools, and
  subprojects.
- **Q9 Flux** is exclusively the emulator. It emulates the Q9 hardware and
  runs Q9-OS. The current technical binary name `q9.exe` will be adjusted
  in a later, planned rename.
- **Q9** resp. **Q9-OS** denotes the operating system.
- **QCC** is the C compiler and one of the project's central deliverables.
- **Parsec** is the parser/grammar generator. Parsec is roughly part of the
  QCC toolchain and supports its frontend/parser generation.
- **QCC backends** contain target-code generation, for example for 68k and
  Q9-OS. Further target platforms may be added.

## Layout

```text
Q9 Forge
├── Q9 Flux             Emulator
├── Q9 / Q9-OS          Operating system
└── QCC toolchain
    ├── Parsec          Parser/grammar generator
    ├── QCC             C compiler
    └── Backends        e.g. 68k for Q9-OS
```

**Q9 Flux**, **Q9-OS**, and **QCC** are thus independent components of
**Q9 Forge**. Parsec is not a second emulator, but a tooling area of the
QCC toolchain.

The spelling **Q9 Flux** is binding; **Q9 Flex** must not be used for the
emulator.

## Multiple target architectures (💡 proposal, 2026-08-11, not yet finally discussed)

So far, Q9 Forge exclusively supports the Motorola 68K family (Q9 Flux as
board emulator, Q9-OS as the guest operating system running inside it).
Andreas wants to extend this to further target architectures: **RISC-V32**,
**ARM64** (the already-started native Q9 port, exact Mac model still open),
and **x86 32-bit Protected Mode** (flat, modeled on OS-9000 for 386+ — real
Real Mode is inherently 16-bit and therefore not a viable basis for a
32-bit kernel).

Each target architecture has two fundamentally different flavors, both
belonging under Q9 Forge but living in different projects/directories:

- **Type A – Board emulator**: full hardware emulation (CPU + board
  peripherals), runs as a host application on Linux/Windows/Mac, runs
  unmodified guest code (e.g. genuine OS-9/68K under Q9 Flux). This is
  **Q9 Flux**'s current design, so far only for Motorola 68K.
- **Type B – native runtime**: Q9-OS itself, natively compiled for the
  target architecture, runs directly on real hardware resp. a real host
  kernel. No CPU/hardware emulation; only a thin HAL translates Q9-OS
  system calls into native host calls (POSIX/Win32/etc.). Corresponds to
  the archived mini-kernel concept (`Q9RESUME-Kernel`, HAL native/posix/wasm).

Names for Type A/Type B, as well as the directory/repo structure per target
architecture, still need to be finalized with Andreas — see `ROADMAP.md`
and `Q9-Flux/ARBEITSPLAN.md` for the current planning status.
