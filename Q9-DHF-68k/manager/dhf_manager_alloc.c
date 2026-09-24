#include "dhf_manager.h"
#include <stdlib.h>
#include <stdio.h>

/* Manager-side allocation helpers.
 * On real OS-9 68K this should call F$SRqMem/F$SRtMem to allocate from the system
 * pool. Here provide stubs that use malloc/free so tests can run in the host.
 */

void *dhf_manager_alloc(size_t size) {
    void *p = malloc(size);
    if (!p) fprintf(stderr, "dhf_manager_alloc: allocation failed (%zu)\n", size);
    return p;
}

void dhf_manager_free(void *p) {
    free(p);
}
