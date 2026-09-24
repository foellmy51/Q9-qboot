#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "../include/dhf_shared.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <emulator_mem_file>\n", argv[0]);
        return 2;
    }
    const char *path = argv[1];
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    struct stat st;
    if (fstat(fd, &st) != 0) {
        perror("fstat");
        close(fd);
        return 1;
    }
    size_t map_sz = (size_t)st.st_size;
    if (map_sz < sizeof(dhf_shared_t)) {
        fprintf(stderr, "file too small for dhf_shared_t (%zu < %zu)\n", map_sz, sizeof(dhf_shared_t));
        close(fd);
        return 1;
    }
    void *mem = mmap(NULL, map_sz, PROT_READ, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }
    dhf_shared_t *sh = (dhf_shared_t *)mem;
    printf("DHF host-simulator (file-backed)\n");
    printf("version: %u, seq: %u, command: 0x%04x\n", sh->version, sh->seq, sh->command);
    for (int i = 0; i < 5; ++i) {
        printf("param[%d]=0x%08x\n", i, (unsigned)sh->param[i]);
    }
    if (munmap(mem, map_sz) != 0) perror("munmap");
    close(fd);
    return 0;
}
