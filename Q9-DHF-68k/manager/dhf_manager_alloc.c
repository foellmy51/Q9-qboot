#include "dhf_manager.h"
#include <stdlib.h>
#include <stdio.h>

/* Manager-side allocation helpers.
 * On real OS-9 68K this should call F$SRqMem/F$SRtMem to allocate from the system
 * pool. Here provide stubs that use malloc/free so tests can run in the host.
 *
 * Additionally export helpers that mimic returning a pointer to be stored in
 * the OS-9 path descriptor (pd->pd_opt) so the driver can access per-path state.
 */

#include <stdint.h>

void *dhf_manager_alloc(size_t size) {
    void *p = malloc(size);
    if (!p) fprintf(stderr, "dhf_manager_alloc: allocation failed (%zu)\n", size);
    return p;
}

void dhf_manager_free(void *p) {
    free(p);
}

/* For host tests, return the pointer value as a 32-bit "emulator address" which
 * manager would have placed in the emulator path descriptor. This helps exercising
 * the driver-manager pointer handoff in tests.
 */
uint32_t dhf_manager_alloc_emulated_addr(size_t size) {
    void *p = dhf_manager_alloc(size);
    if (!p) return 0;
    uintptr_t v = (uintptr_t)p;
    return (uint32_t)(v & 0xffffffff);
}

void *dhf_manager_resolve_emulated_addr(uint32_t addr) {
    if (addr == 0) return NULL;
    return (void *)(uintptr_t)addr;
}
