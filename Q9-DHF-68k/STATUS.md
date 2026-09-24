# Q9-DHF-68k Status

| Component | Function / Item | Status | Notes |
|---|---|---:|---|
| Manager | init | 🟡 Partially | init forwards to driver init; further validation needed |
| Manager | open/close/read/write | 🟡 Partially | Forwarding stubs present; host-simulator and driver stubs exercise basic flow (open/create/read/write/close implemented in host-sim) |
| Manager | getstat/setstat | 🟡 Partially | Host-sim supports GetStat; SetStat implemented (chmod only) — further attribute mapping needed |
| Manager | chdir/cd | 🟡 Partially | Manager and host-sim implement path handling; ensure chdir doesn't escape basepath — needs tests and manager validation |
| Manager | mkdir/rmdir/unlink | ❌ Not started | |
| Manager | rename | ❌ Not started | |
| Manager | opendir/readdir | ✅ Done (basic) | Opendir, Readdir and handle tracking implemented for up to 16 DIR handles; supports single-entry reads per call |
| Driver (dhfdrv-68k) | init | 🟡 Partially | Driver stub exists; host-sim provides host FS mounting and confinement helpers |
| Driver (dhfdrv-68k) | open/read/write/close | 🟡 Partially | Implemented mapping to host FS ops (open/read/write/close); needs error mapping and PD usage validation |
| Descriptor | basepath getter/setter | ✅ Done | Getter/setter implemented in descriptor/dhf_descriptor.c; host simulator uses basepath for confinement |

> Legend: ❌ Not started, 🟡 Partially/rudimentary, ✅ Done

