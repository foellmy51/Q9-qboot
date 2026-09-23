#include "q9_bios.h"
#include "qboot_hal.h"
#include <string.h>

/* Mock-Hardware-Deskriptoren für das Standard-System */

static struct q9_scf_descriptor uart_desc = {
    .header = {
        .class_id = Q9_DEV_CLASS_SCF,
        .hw_type = Q9_HW_TYPE_UART_16550,
        .name = "/term0",
        .port_address = 0xFFF00000, /* Beispieladresse für I/O-Ports */
        .irq_vector = 64,
        .irq_priority = 4
    },
    .baudrate = 115200,
    .parity = 0,          /* Keine */
    .stop_bits = 1,
    .data_bits = 8,
    .flow_control = 0     /* Keine */
};

static struct q9_rbf_descriptor sd_desc = {
    .header = {
        .class_id = Q9_DEV_CLASS_RBF,
        .hw_type = Q9_HW_TYPE_SD_CARD,
        .name = "/h0",
        .port_address = 0xFFF10000,
        .irq_vector = 65,
        .irq_priority = 3
    },
    .block_size = 512,
    .total_blocks = 31250000, /* ~16 GB SD-Karte */
    .partition_type = 0x0C,    /* FAT32 (LBA) */
    .fs_format = 2,            /* FAT32 */
    .is_bootable = 1
};

/* Interne Deskriptor-Registrierungsliste des BIOS */
static struct q9_dev_header *system_descriptors[] = {
    (struct q9_dev_header *)&uart_desc,
    (struct q9_dev_header *)&sd_desc
};
#define NUM_DESCRIPTORS (sizeof(system_descriptors) / sizeof(system_descriptors[0]))

/* HAL-Referenzen für aktive Komponenten */
extern struct console_ops uart_console_ops;
static struct console_device active_console = {
    .name = "/term0",
    .ops = &uart_console_ops,
    .priv = (void *)0xFFF00000
};

/* --- BIOS API-Implementierungen --- */

static int bios_get_descriptors(struct q9_dev_header *dest_array, int max_count) {
    if (!dest_array || max_count <= 0) {
        return 0;
    }

    int copy_count = (max_count < (int)NUM_DESCRIPTORS) ? max_count : (int)NUM_DESCRIPTORS;
    for (int i = 0; i < copy_count; i++) {
        size_t size = 0;
        if (system_descriptors[i]->class_id == Q9_DEV_CLASS_SCF) {
            size = sizeof(struct q9_scf_descriptor);
        } else if (system_descriptors[i]->class_id == Q9_DEV_CLASS_RBF) {
            size = sizeof(struct q9_rbf_descriptor);
        } else {
            size = sizeof(struct q9_dev_header);
        }
        
        /* Kopiert den Deskriptor byte-genau in den Zielspeicher */
        memcpy((uint8_t *)dest_array + (i * sizeof(struct q9_dev_header)), system_descriptors[i], size);
    }
    return copy_count;
}

static int bios_console_init(struct q9_scf_descriptor *desc) {
    if (!desc) {
        return Q9_BIOS_ERR_INVALID_VAL;
    }

    /* Setze die aktive Konsole auf die neue Registeradresse */
    active_console.priv = (void *)(uintptr_t)desc->header.port_address;
    
    /* Falls ein anderer Treiber (z.B. Framebuffer) gewählt wurde, hier umschalten.
     * Für Demo-Zwecke nutzen wir den UART-Treiber.
     */
    if (active_console.ops && active_console.ops->init) {
        return active_console.ops->init(active_console.priv);
    }
    return Q9_BIOS_ERR_GENERIC;
}

static void bios_console_putc(char c) {
    if (active_console.ops && active_console.ops->putc) {
        active_console.ops->putc(active_console.priv, c);
    }
}

static char bios_console_getc(void) {
    if (active_console.ops && active_console.ops->getc) {
        return active_console.ops->getc(active_console.priv);
    }
    return 0;
}

static int bios_console_tstc(void) {
    if (active_console.ops && active_console.ops->tstc) {
        return active_console.ops->tstc(active_console.priv);
    }
    return 0;
}

static int bios_storage_init(struct q9_rbf_descriptor *desc) {
    if (!desc) {
        return Q9_BIOS_ERR_INVALID_VAL;
    }
    /* Bindet und initialisiert die Speicherhardware.
     * In einem echten System wird je nach desc->header.hw_type der SD- oder SPI-Treiber aufgerufen.
     */
    return Q9_BIOS_SUCCESS;
}

static int bios_storage_info(struct q9_rbf_descriptor *desc) {
    if (!desc) {
        return Q9_BIOS_ERR_INVALID_VAL;
    }
    /* Auto-Erkennung von Sektorengrößen und Medienpräsenz.
     * Hier geben wir feste Beispieldaten der erkannten Karte zurück.
     */
    desc->block_size = 512;
    desc->total_blocks = 31250000;
    desc->fs_format = 2; /* Erkanntes FAT32 */
    desc->is_bootable = 1;
    return Q9_BIOS_SUCCESS;
}

static int bios_storage_read(struct q9_rbf_descriptor *desc, uint32_t sector, uint32_t count, void *buffer) {
    if (!desc || !buffer) {
        return Q9_BIOS_ERR_INVALID_VAL;
    }
    
    /* Hier würde der Aufruf an die physikalische SD-Karten-HAL-Schnittstelle erfolgen. */
    (void)sector;
    (void)count;
    return Q9_BIOS_SUCCESS;
}

static int bios_storage_write(struct q9_rbf_descriptor *desc, uint32_t sector, uint32_t count, const void *buffer) {
    if (!desc || !buffer) {
        return Q9_BIOS_ERR_INVALID_VAL;
    }

    /* Hier würde der Schreibaufruf an die SD-Karten-HAL erfolgen. */
    (void)sector;
    (void)count;
    return Q9_BIOS_SUCCESS;
}

static void bios_system_reset(void) {
    /* Hard-Reset Register triggern (z.B. Watchdog-Reset oder System Control Register).
     * Hier simulieren wir einen endlosen Halt als Fallback.
     */
    while (1) {
        /* Halt */
    }
}

/* --- Globale BIOS-Vektortabelle --- */
const struct q9_bios_vectors bios_vector_table = {
    .bios_version = 0x00010000, /* v1.0.0 */
    .qumips_rating = 0,         /* Wird durch Kalibrierung gesetzt */

    .get_descriptors = bios_get_descriptors,

    .console_init = bios_console_init,
    .console_putc = bios_console_putc,
    .console_getc = bios_console_getc,
    .console_tstc = bios_console_tstc,

    .storage_init = bios_storage_init,
    .storage_info = bios_storage_info,
    .storage_read = bios_storage_read,
    .storage_write = bios_storage_write,

    .system_reset = bios_system_reset
};
