# Q9 Forge – Roadmap

The roadmap describes the planned development of Q9 Forge and its subprojects. Dates are deliberately not fixed; the order follows dependencies and stable intermediate states.

*German version: [ROADMAP_de.md](ROADMAP_de.md)*

v    ready, maybe with some issues
-    in progress, actively being developed now
x    planned, tomorrow, next month, or next century :-)


## Q9 OS

- Phase 1

    - Kernel reverse engineering, disassembly              - Dec 2026
    - Kernel planning: which submodules do we need         x 2027
    - Kernel: first translation of an existing kernel      x 2027
      using the original code
    - Kernel: rewrite in C code                            x 2027
    - Build system modules                                 x 2027/28
    - Build manager, investigate and plan                  x 2028


## Q9 Flux Emulator

- Phase 1
    - Emulator for Motorola 68K (Musashi-based)             v
      running on Mac, Linux, Windows
    - add hardware simulation for real or virtual hardware  v
    - add port for OS-9/68k as 68030 CPU                    v
    - add virtual network terminal simulators               v
    - add network support as Ethernet simulation            v
    - add telnet support in OS-9                            v
    - add framebuffer support with remote support           - Aug 2026

- Phase 2
    - add new virtual fast hardware target for Q9 Flux      x 2027
    - add new target hardware mc68000                       x 2027
    - add internal emulator 6809                            x 2027
    - add internal CP/M-68k emulation                       x 2027

- Phase 3 -- multiple target architectures (proposal 2026-08-11, details see
  Q9Forge/AI_CONTEXT.md "Multiple target architectures" and Q9-Flux/ARBEITSPLAN.md)

    Two flavors per target architecture: Type A (board emulator, as today
    for 68K) and Type B (native Q9-OS runtime without hardware emulation,
    syscalls go to the real host kernel).

    - add new target RISC-V32 (Type A + Type B)             x 2028
    - add new target ARM64 (Type B first -- native Q9 port
      already started, Mac model open; Type A optional
      later)                                                x 2028
    - add new target x86 32-bit Protected Mode, flat
      (modeled on OS-9000 for 386+, NOT Real Mode -- that
      is inherently 16-bit; Type A + Type B)                x 2028
    - add new target Raspberry (ARM, Type A)                x 2029




## Q9 QCC  C Compiler

- Phase 1
    - EBNF parser, create recursive descent parser          v   (first version)
    - Code generation to intermediate code                  v
    - Intermediate code defined as a stack machine          v
    - Backend for 68k from intermediate code                v

- Phase 2
    - C preprocessor                                        x 2027
    - 68k assembler for Q9                                  x 2027
    - Linker for Q9 ROF format                              x 2028

- Phase 3
    - other frontends, Rust                                 x 2028
    - Interpreter for intermediate code                     x 2028


## Vinculum
   Hardware with Motorola 68360,
       network,
       32 MByte RAM
       dual CF drive
       USB stick

- Phase 1
       Schematic                                            - 2027
       Order PCB and devices                                x 2028
       First prototype with running Q9                      x 2028



## Phase 1 – Organize project foundation

- Establish Q9 Forge as the umbrella structure
- Cleanly position Q9 Flux and Q9 Frame
- Inventory existing projects and build paths
- Define shared naming conventions and documentation structure
- Prepare the public repository structure

## Phase 2 – Stabilize Q9 Flux

- Clearly separate emulator and hardware-simulation areas
- Document memory, I/O, and device map
- Unify existing tests and build variants
- Ensure reproducible builds for the supported platforms

## Phase 3 – Q9 Frame v1

- Connect separate video RAM at `0xFD000000`
- Integrate the MC6845 register model starting at `0xFFFFA000`
- Implement a monochrome 1-bit framebuffer
- Define and test bit and byte layout
- Implement dirty tracking on VRAM writes

## Phase 4 – Host service manager

- Provide a shared, non-blocking service framework
- Integrate the existing terminal service
- Add the Q9 Frame TCP service
- Implement update rate, send queues, and client lifecycle

## Phase 5 – Q9 Frame network v1

- `HELLO` and `VIDEO_INFO`
- `FRAME_FULL` and `FRAME_UPDATE`
- Full-frame request and resynchronization
- UDP discovery on the local network
- Protocol and connection tests

## Phase 6 – Test clients

- Small desktop reference client
- Nearest-neighbor scaling
- Windowed, fullscreen, and minimized behavior
- Testing with various update rates and slow clients
- Later: ESP32 client with a small display

## Phase 7 – Public release

- Complete the README and quickstart
- Document licenses and third-party components
- Remove debug data, local paths, and private configs
- Set up CI, tests, and reproducible builds
- Define versioning and release convention
- First public development snapshot of Q9 Forge

## Phase 8 – Extensions

- Color modes and virtual DAC models
- Multiple simultaneous video clients
- Optional UDP transport for video updates
- Keyboard and mouse channel
- Configurable pixel and memory formats
- Further MC6845 timing and cursor functions
- Later window and terminal integration
