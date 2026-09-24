/* dhfdrv-68k: driver skeleton mapping to host filesystem
 * Implement platform-specific host FS operations and confinement here.
 */

#include "dhfdrv-68k.h"
#include <string.h>
#include <stdio.h>

int dhfdrv_init(const char *basepath) { (void)basepath; /* TODO: init host FS mapping */ return 0; }

/* Attach a manager-allocated per-path control block emulated-address to driver
 * state. In the real system manager will store a pointer in the path descriptor
 * (pd->pd_opt) which the driver can read. For our tests the manager will call
 * dhf_manager_alloc_emulated_addr() and pass the 32-bit value into the driver
 * via higher-level wiring; the driver can resolve it below.
 */

#include <stdint.h>

void *dhfdrv_resolve_pd_from_emulated_addr(uint32_t emu_addr) {
    return dhf_manager_resolve_emulated_addr(emu_addr);
}

int dhfdrv_open(const char *path, int flags) { (void)path; (void)flags; return -1; }
int dhfdrv_close(int fd) { (void)fd; return -1; }
ssize_t dhfdrv_read(int fd, void *buf, size_t count) { (void)fd; (void)buf; (void)count; return -1; }
ssize_t dhfdrv_write(int fd, const void *buf, size_t count) { (void)fd; (void)buf; (void)count; return -1; }
int dhfdrv_getstat(const char *path, void *statbuf) { (void)path; (void)statbuf; return -1; }
int dhfdrv_setstat(const char *path, void *statbuf) { (void)path; (void)statbuf; return -1; }
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int dhfdrv_chdir(const char *path) {
    char base[PATH_MAX];
    const char *bp = dhf_descriptor_get_basepath();
    if (!bp) return -1;
    strncpy(base, bp, sizeof(base)-1);
    base[sizeof(base)-1] = '\0';

    // Build candidate path under base
    char candidate[PATH_MAX];
    if (path[0] == '/')
        snprintf(candidate, sizeof(candidate), "%s%s", base, path);
    else
        snprintf(candidate, sizeof(candidate), "%s/%s", base, path);

    // Resolve realpaths and ensure confinement
    char real_base[PATH_MAX];
    char real_cand[PATH_MAX];
    if (!realpath(base, real_base)) return -1;
    if (!realpath(candidate, real_cand)) return -1;
    size_t lb = strlen(real_base);
    if (strncmp(real_base, real_cand, lb) != 0) {
        errno = EACCES;
        return -1;
    }

    // perform chdir to the real candidate
    if (chdir(real_cand) != 0) return -1;
    return 0;
}
int dhfdrv_mkdir(const char *path, int mode) { (void)path; (void)mode; return -1; }
int dhfdrv_rmdir(const char *path) { (void)path; return -1; }
int dhfdrv_unlink(const char *path) { (void)path; return -1; }
int dhfdrv_rename(const char *oldp, const char *newp) { (void)oldp; (void)newp; return -1; }
int dhfdrv_opendir(const char *path) { (void)path; return -1; }
int dhfdrv_readdir(int dirfd, void *entry) { (void)dirfd; (void)entry; return -1; }

