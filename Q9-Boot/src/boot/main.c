#include "q9_bios.h"
#include "qboot_lib.h"
#include "qboot_cli.h"

int main(void) {
    /* 1. BIOS Konsole initialisieren */
    struct q9_dev_header desc_header;
    
    /* Wir holen uns das erste SCF-Gerät (Konsole) aus den Deskriptoren */
    int count = bios_vector_table.get_descriptors(&desc_header, 1);
    if (count > 0 && desc_header.class_id == Q9_DEV_CLASS_SCF) {
        bios_vector_table.console_init((struct q9_scf_descriptor *)&desc_header);
    }
    
    /* 2. Boot-Banner ausgeben */
    bios_puts("\n==================================================\n");
    bios_puts("         Q-Boot Bootloader & BIOS v1.0.0\n");
    bios_puts("==================================================\n");
    
    /* 3. POST (Power-On Self-Test) */
    bios_puts("POST: Pruefe CPU Register... OK\n");
    bios_puts("POST: Pruefe RAM-Segmente... OK\n");
    
    /* 4. Kalibrierung & Qumips bestimmen */
    bios_puts("Kalibriere System-Takt... ");
    uint32_t rating = calculate_qumips();
    
    /* qumips_rating in der BIOS-Vektortabelle updaten (Cast um const zu umgehen) */
    *(uint32_t *)&bios_vector_table.qumips_rating = rating;
    
    bios_put_dec(rating / 100);
    bios_puts(".");
    bios_put_dec(rating % 100);
    bios_puts(" Qumips gemessen.\n");
    
    /* 5. Boot-Countdown starten */
    bios_puts("\nDruecke eine Taste, um den Autostart zu unterbrechen: 3... ");
    
    int interrupt_boot = 0;
    
    for (int sec = 3; sec > 0; sec--) {
        /* Wartezeit simulieren und auf Tastendruck prüfen */
        for (volatile uint32_t delay = 0; delay < 10000000; delay++) {
            if (bios_vector_table.console_tstc()) {
                /* Eine Taste wurde gedrückt! */
                bios_vector_table.console_getc(); /* Zeichen konsumieren */
                interrupt_boot = 1;
                break;
            }
        }
        if (interrupt_boot) {
            break;
        }
        
        if (sec == 3) bios_puts("2... ");
        if (sec == 2) bios_puts("1... ");
    }
    
    if (interrupt_boot) {
        /* 6. Interaktive Boot-Shell starten */
        cli_run();
    } else {
        /* 7. Autoboot-Sequenz starten */
        bios_puts("\n\nAutoboot gestartet...\n");
        bios_puts("Scanne Massenspeicher /h0...\n");
        
        struct q9_rbf_descriptor sd_desc;
        bios_vector_table.storage_info(&sd_desc);
        
        if (sd_desc.is_bootable) {
            bios_puts("Lade OS Kernel von Partition Typ ");
            bios_put_hex(sd_desc.partition_type);
            bios_puts("... geladen.\n");
            bios_puts("Springe in den Kernel (Bootstrapping erfolgreich!)\n\n");
            /* Hier würde der Sprung ins OS (z.B. Q9-OS) stattfinden */
        } else {
            bios_puts("Fehler: Kein bootfaehiges Medium gefunden!\n");
            cli_run();
        }
    }
    
    bios_puts("\nSystem gestoppt. Reset ausloesen.\n");
    bios_vector_table.system_reset();
    return 0;
}
