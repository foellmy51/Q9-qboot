#ifndef QBOOT_LIB_H
#define QBOOT_LIB_H

#include <stdint.h>

/**
 * @brief Gibt eine null-terminierte Zeichenkette auf der BIOS-Konsole aus.
 */
void bios_puts(const char *str);

/**
 * @brief Gibt eine 32-Bit-Zahl als Hexadezimalwert (z.B. 0x00FAB123) aus.
 */
void bios_put_hex(uint32_t val);

/**
 * @brief Gibt eine 32-Bit-Zahl als Dezimalwert aus.
 */
void bios_put_dec(uint32_t val);

/**
 * @brief Führt die Qumips-Kalibrierungsroutine aus und ermittelt die Rechenleistung.
 * @return Die berechneten Qumips (z.B. 1250 für 12.5 Qumips).
 */
uint32_t calculate_qumips(void);

#endif /* QBOOT_LIB_H */
