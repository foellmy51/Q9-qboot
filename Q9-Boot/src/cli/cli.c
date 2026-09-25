#include "qboot_cli.h"
#include "q9_bios.h"
#include "qboot_lib.h"
#include <string.h>

#define MAX_LINE_LEN  80
#define MAX_ARGS      8

/* Einfache Mock-Umgebungsvariablen für das Hardware-Setup */
struct env_var {
    const char *key;
    char value[32];
};

static struct env_var setup_env[] = {
    { "bootdelay", "3" },
    { "bootorder", "/h0,/term0" },
    { "baudrate",  "115200" },
    { "videomode", "800x600x32" }
};
#define NUM_ENV (sizeof(setup_env) / sizeof(setup_env[0]))

/* Hilfsfunktion zum Vergleichen von Zeichenketten */
static int strcmp_local(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

/* Einfache String-zu-Hex-Konvertierung */
static uint32_t parse_hex(const char *str) {
    uint32_t val = 0;
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
    }
    while (*str) {
        char c = *str++;
        val <<= 4;
        if (c >= '0' && c <= '9') {
            val |= (c - '0');
        } else if (c >= 'A' && c <= 'F') {
            val |= (c - 'A' + 10);
        } else if (c >= 'a' && c <= 'f') {
            val |= (c - 'a' + 10);
        } else {
            val >>= 4; /* Ungültiges Zeichen, abbrechen */
            break;
        }
    }
    return val;
}

/* --- Befehls-Implementierungen --- */

static void cmd_help(void) {
    bios_puts("\nVerfuegbare Befehle:\n");
    bios_puts("  help                      - Zeigt diese Hilfe an\n");
    bios_puts("  info                      - Zeigt System- und Deskriptor-Informationen\n");
    bios_puts("  md <addr>                 - Memory Dump: Zeigt 16 Bytes ab Adresse (Hex)\n");
    bios_puts("  mw <addr> <val>           - Memory Write: Schreibt 32-Bit Wert in Adresse\n");
    bios_puts("  printenv                  - Listet alle Setup-Variablen auf\n");
    bios_puts("  setenv <key> <val>        - Setzt eine Setup-Variable im Memory\n");
    bios_puts("  boot                      - Startet das Betriebssystem (Autoboot)\n");
    bios_puts("  reset                     - Startet das System neu\n");
}

static void cmd_info(void) {
    bios_puts("\n--- Q-Boot System Information ---\n");
    bios_puts("BIOS Version: ");
    bios_put_hex(bios_vector_table.bios_version);
    bios_puts("\nCPU Leistung: ");
    bios_put_dec(bios_vector_table.qumips_rating / 100);
    bios_puts(".");
    bios_put_dec(bios_vector_table.qumips_rating % 100);
    bios_puts(" Qumips\n");

    bios_puts("\n--- Erkannte Hardware-Deskriptoren ---\n");
    struct q9_dev_header descriptors[8];
    int count = bios_vector_table.get_descriptors(descriptors, 8);
    
    for (int i = 0; i < count; i++) {
        bios_puts("Geraet [");
        bios_put_dec(i);
        bios_puts("]: ");
        bios_puts(descriptors[i].name);
        bios_puts(" (Klasse: ");
        bios_put_dec(descriptors[i].class_id);
        bios_puts(", Typ: ");
        bios_put_dec(descriptors[i].hw_type);
        bios_puts(", Port: ");
        bios_put_hex(descriptors[i].port_address);
        bios_puts(")\n");
    }
}

static void cmd_md(const char *addr_str) {
    if (!addr_str) {
        bios_puts("\nFehler: Adresse fehlt! Syntax: md <addr>\n");
        return;
    }
    uint32_t addr = parse_hex(addr_str);
    volatile uint8_t *ptr = (volatile uint8_t *)(uintptr_t)addr;
    
    bios_puts("\n");
    bios_put_hex(addr);
    bios_puts(": ");
    for (int i = 0; i < 16; i++) {
        uint8_t byte = 0;
        #ifdef BARE_METAL
            byte = ptr[i];
        #else
            byte = ((uint8_t*)&bios_vector_table)[(addr + i) % sizeof(bios_vector_table)];
        #endif
        
        char hex_chars[] = "0123456789ABCDEF";
        bios_vector_table.console_putc(hex_chars[byte >> 4]);
        bios_vector_table.console_putc(hex_chars[byte & 0x0F]);
        bios_vector_table.console_putc(' ');
    }
    bios_puts("\n");
}

static void cmd_mw(const char *addr_str, const char *val_str) {
    if (!addr_str || !val_str) {
        bios_puts("\nFehler: Argumente fehlen! Syntax: mw <addr> <val>\n");
        return;
    }
    uint32_t addr = parse_hex(addr_str);
    uint32_t val = parse_hex(val_str);
    volatile uint32_t *ptr = (volatile uint32_t *)(uintptr_t)addr;
    
    #ifdef BARE_METAL
        *ptr = val;
    #endif
    
    bios_puts("\nSchreibe ");
    bios_put_hex(val);
    bios_puts(" nach ");
    bios_put_hex(addr);
    bios_puts("... OK\n");
}

static void cmd_printenv(void) {
    bios_puts("\n--- Q-Boot Setup Environment ---\n");
    for (size_t i = 0; i < NUM_ENV; i++) {
        bios_puts(setup_env[i].key);
        bios_puts("=");
        bios_puts(setup_env[i].value);
        bios_puts("\n");
    }
}

static void cmd_setenv(const char *key, const char *val) {
    if (!key || !val) {
        bios_puts("\nFehler: Syntax: setenv <key> <value>\n");
        return;
    }
    for (size_t i = 0; i < NUM_ENV; i++) {
        if (strcmp_local(setup_env[i].key, key) == 0) {
            /* Wert kopieren */
            size_t len = 0;
            while (val[len] && len < 31) {
                setup_env[i].value[len] = val[len];
                len++;
            }
            setup_env[i].value[len] = '\0';
            bios_puts("\nVariable gesetzt.\n");
            return;
        }
    }
    bios_puts("\nFehler: Unbekannte Variable. Kann im statischen Setup nicht neu angelegt werden.\n");
}

/* --- Haupt-CLI-Schleife --- */

void cli_run(void) {
    char line[MAX_LINE_LEN];
    int line_idx = 0;
    
    bios_puts("\n*** Willkommen in der Q-Boot Shell ***\n");
    bios_puts("Tippe 'help' fuer eine Liste der Befehle.\n");
    
    while (1) {
        bios_puts("\nQ-Boot> ");
        line_idx = 0;
        
        while (1) {
            char c = bios_vector_table.console_getc();
            
            /* Handle Enter / Newline */
            if (c == '\r' || c == '\n') {
                line[line_idx] = '\0';
                break;
            }
            /* Handle Backspace (ASCII 8 oder 127) */
            else if (c == 8 || c == 127) {
                if (line_idx > 0) {
                    line_idx--;
                    bios_puts("\b \b"); /* Cursor zurueck, Leerzeichen, Cursor zurueck */
                }
            }
            /* Handle normale Zeichen */
            else if (line_idx < MAX_LINE_LEN - 1) {
                line[line_idx++] = c;
                bios_vector_table.console_putc(c); /* Echo an den User */
            }
        }
        
        /* Tokenizer (Einfaches Trennen nach Leerzeichen) */
        char *argv[MAX_ARGS];
        int argc = 0;
        char *token = line;
        
        /* Überspringe führende Leerzeichen */
        while (*token == ' ') token++;
        
        while (*token && argc < MAX_ARGS) {
            argv[argc++] = token;
            while (*token && *token != ' ') token++;
            if (*token) {
                *token = '\0';
                token++;
                while (*token == ' ') token++;
            }
        }
        
        if (argc == 0) continue;
        
        /* Befehlsauswertung */
        if (strcmp_local(argv[0], "help") == 0) {
            cmd_help();
        } else if (strcmp_local(argv[0], "info") == 0) {
            cmd_info();
        } else if (strcmp_local(argv[0], "md") == 0) {
            cmd_md(argc > 1 ? argv[1] : NULL);
        } else if (strcmp_local(argv[0], "mw") == 0) {
            cmd_mw(argc > 1 ? argv[1] : NULL, argc > 2 ? argv[2] : NULL);
        } else if (strcmp_local(argv[0], "printenv") == 0) {
            cmd_printenv();
        } else if (strcmp_local(argv[0], "setenv") == 0) {
            cmd_setenv(argc > 1 ? argv[1] : NULL, argc > 2 ? argv[2] : NULL);
        } else if (strcmp_local(argv[0], "boot") == 0) {
            bios_puts("\nStarte OS Bootvorgang von /h0...\n");
            break;
        } else if (strcmp_local(argv[0], "reset") == 0) {
            bios_puts("\nStarte System neu...\n");
            bios_vector_table.system_reset();
        } else {
            bios_puts("\nUnbekannter Befehl: ");
            bios_puts(argv[0]);
            bios_puts("\nTippe 'help' fuer Hilfe.\n");
        }
    }
}
