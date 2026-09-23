#include "qboot_lib.h"
#include "q9_bios.h"

/* Host/build stubs for tick and scheduler used by q9_bogomips when building native qboot */
unsigned long read_ticks(void) { return 0UL; }
void scheduler_lock(void) { }
void scheduler_unlock(void) { }

void bios_puts(const char *str) {
    if (!str) {
        return;
    }
    while (*str) {
        /* Nutzt den BIOS-Vektor für die Zeichenausgabe */
        bios_vector_table.console_putc(*str++);
    }
}

void bios_put_hex(uint32_t val) {
    char hex_chars[] = "0123456789ABCDEF";
    bios_puts("0x");
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (val >> (i * 4)) & 0x0F;
        bios_vector_table.console_putc(hex_chars[nibble]);
    }
}

void bios_put_dec(uint32_t val) {
    char buf[11];
    int i = 10;
    buf[i] = 0;
    
    if (val == 0) {
        bios_vector_table.console_putc(0);
        return;
    }
    
    while (val > 0) {
        buf[--i] = 0 + (val % 10);
        val /= 10;
    }
    bios_puts(&buf[i]);
}

uint32_t calculate_qumips(void) {
    /* Kalibrierungs-Benchmark-Simulation.
     * In echter Hardware würde hier eine Schleife gegen einen Hardware-Timer laufen,
     * um die MIPS (Millionen Instruktionen pro Sekunde) der CPU zu bestimmen.
     */
    volatile uint32_t delay_loop;
    for (delay_loop = 0; delay_loop < 100000; delay_loop++) {
        /* Benchmark-Schleife */
    }
    
    /* Gibt 1250 Qumips zurück (entspricht 12.5 Qumips, da wir mit Faktor 100 arbeiten) */
    return 1250; 
}
