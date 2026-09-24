#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>

#include "../include/dhf_shared.h"

static volatile int keep_running = 1;
static void sigint_handler(int sig) { (void)sig; keep_running = 0; }

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <emulator_mem_file>\n", argv[0]);
        return 2;
    }
    signal(SIGINT, sigint_handler);

    const char *path = argv[1];
    int fd = open(path, O_RDWR);
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
    void *mem = mmap(NULL, map_sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }
    dhf_shared_t *sh = (dhf_shared_t *)mem;

    printf("DHF host-simulator (file-backed) started. Monitoring commands...\n");
    uint32_t last_seq = sh->seq;
    uint16_t last_cmd = sh->command;

    while (keep_running) {
        uint32_t seq = sh->seq;
        uint16_t cmd = sh->command;
        if (cmd != 0 && (cmd < 0x0080)) { // request range
            printf("Detected command 0x%04x seq=%u\n", cmd, seq);
            for (int i = 0; i < 5; ++i) printf(" param[%d]=0x%08x\n", i, (unsigned)sh->param[i]);

            // Basic handlers
            if (cmd == 0x0021) { // Descriptor Init
                printf("Handling Descriptor Init\n");
                // For now, just acknowledge
                sh->result_code = 0; // ok
            } else if (cmd == 0x0022) { // Ping
                printf("Handling Ping\n");
                sh->result_code = 0; // ok
            } else {
                printf("Unhandled command 0x%04x - echoing OK\n", cmd);
                sh->result_code = 0; // ok
            }

            // Set response command (0x0080 == OK)
            sh->command = 0x0080;

            // bump seq to indicate update (simple approach)
            sh->seq = seq + 1;

            // wait for manager to clear or change command
            int waitcnt = 0;
            while (sh->command == 0x0080 && keep_running && waitcnt++ < 1000) {
                usleep(1000);
            }
        }
        usleep(1000*50);
    }

    if (munmap(mem, map_sz) != 0) perror("munmap");
    close(fd);
    printf("DHF host-simulator exiting.\n");
    return 0;
}
