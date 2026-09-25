q9_bogomips Integration Guide

Overview

This document describes how q9_bogomips was integrated into the Q9-Boot project, how to build the native simulator, how to perform a baremetal build (cross-compile), and how to run the ROM image in the Q9-Flux emulator.

Files added/changed
- src/bios/q9_bogomips.c - calibration routine (Bogomips-style) added to BIOS sources
- src/lib/lib_core.c - host stubs for read_ticks/scheduler_lock/unlock to allow native build
- Makefile - SRCS updated to include src/bios/q9_bogomips.c
- feature branch: feature/bogomips (remote)

Building native qboot (host simulator)
1. From the Q9-Boot root:
   make clean && make
2. The native qboot binary will be produced as ./qboot. It contains the q9_bogomips code and uses host stubs; note that the native simulator does not perform the real hardware tick calibration.

Baremetal build (m68k)
1. Ensure you have an m68k cross toolchain available. Q9-Boot expects a CROSS_COMPILE prefix. Example: CROSS_COMPILE=m68k-elf-
2. Build command (example):
   make baremetal CROSS_COMPILE=m68k-elf- BOARD=cb030
3. Output files: qboot_cb030.elf and qboot_cb030.bin (binary ROM image).

Notes about toolchain
- If the CROSS_COMPILE: m68k-elf- is not in PATH (i.e., m68k-elf-as missing), either install or provide the correct prefix/path (e.g. /opt/q9-toolchain/bin/m68k-q9-).
- The Makefile calls $(CROSS_COMPILE)objcopy to convert ELF to binary. Ensure objcopy is available.

Integrating calibration into ROM/BIOS runtime
- q9_bogomips.c provides the function:
    unsigned long calibrate_loops_per_jiffy(void);
  which depends on platform implementations of:
    unsigned long read_ticks(void);    // must return a monotonically increasing tick counter incremented at HZ (100Hz)
    void scheduler_lock(void);         // prevent context switches but leave timer IRQs running
    void scheduler_unlock(void);
- On real hardware, implement these hooks in the BIOS/board-specific code so the routine can run before the scheduler starts.

Running in Q9-Flux emulator
1. Build baremetal ROM (see above) to get qboot_cb030.bin.
2. Start Q9-Flux with the ROM image, example (adjust paths):
   Q9-Flux-68k --rom /path/to/qboot_cb030.bin
3. Observe boot messages and calibration output printed by BIOS.

Repository
- Changes are pushed to remote branch: feature/bogomips
- Remote: git@github.com:foellmy51/Q9-qboot.git

If you want, I can:
- Add platform hook stubs for a specific board in src/bios/ to call calibrate_loops_per_jiffy at boot.
- Implement more realistic host tick emulation for the simulator binary.
- Attempt to run Q9-Flux with the produced ROM once a baremetal binary is produced.

End of document
