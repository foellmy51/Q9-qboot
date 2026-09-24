# Q9-DHF-68k Status

| Component | Function / Item | Status | Notes |
|---|---|---:|---|
| Manager | init | 🟡 Partially | init forwards to driver init; further validation needed |
| Manager | open/close/read/write | 🟡 Partially | Forwarding stubs present; host-simulator and driver stubs exercise basic flow (open/create/read/write/close implemented in host-sim) |
| Manager | getstat/setstat | 🟡 Partially | Host-sim supports GetStat; SetStat planned (writes to host via st_mode/st_mtime mapping) |
| Manager | chdir/cd | ❌ Not started | Must enforce basepath confinement; host-sim will handle but manager changes must be validated |
| Manager | mkdir/rmdir/unlink | ❌ Not started | |
| Manager | rename | ❌ Not started | |
| Manager | opendir/readdir | 🟡 Partially | Opendir returns a stub handle; Readdir planned (needs DIR* tracking) |
| Driver (dhfdrv-68k) | init | 🟡 Partially | Driver stub exists; host-sim provides host FS mounting and confinement helpers |
| Driver (dhfdrv-68k) | open/read/write/close | 🟡 Partially | Host-simulator implements host-side FS operations; driver skeleton present but integration with emulator manager still needed |
| Descriptor | basepath getter/setter | ✅ Done | Getter/setter implemented in descriptor/dhf_descriptor.c; host simulator uses basepath for confinement |

> Legend: ❌ Not started, 🟡 Partially/rudimentary, ✅ Done

