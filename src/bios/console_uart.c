#include "qboot_hal.h"

/* Registerspezifische Offsets für einen Standard 16550 UART */
#define UART_THR_REG   0  /* Transmitter Holding Register (Write) */
#define UART_RHR_REG   0  /* Receiver Buffer Register (Read) */
#define UART_LSR_REG   5  /* Line Status Register */

#define UART_LSR_DR    0x01  /* Data Ready */
#define UART_LSR_THRE  0x20  /* Transmitter Holding Register Empty */

static int uart_init(void *priv) {
    volatile uint8_t *base = (volatile uint8_t *)priv;
    if (!base) {
        return -1;
    }

    /* Hier würde die Initialisierung stattfinden:
     * - Baudrate einstellen (Baud-Generator Register)
     * - Parameter setzen (8 Datenbits, keine Parität, 1 Stoppbit - 8N1)
     * - FIFOs aktivieren
     */
    return 0;
}

static void uart_putc(void *priv, char c) {
    volatile uint8_t *base = (volatile uint8_t *)priv;
    if (!base) {
        return;
    }

    /* Warten, bis das Sende-Register leer ist (THRE bit) */
    while ((base[UART_LSR_REG] & UART_LSR_THRE) == 0) {
        /* Busy Wait */
    }

    /* Zeichen in Sende-Register schreiben */
    base[UART_THR_REG] = c;
}

static char uart_getc(void *priv) {
    volatile uint8_t *base = (volatile uint8_t *)priv;
    if (!base) {
        return 0;
    }

    /* Warten, bis ein Zeichen empfangen wurde (DR bit) */
    while ((base[UART_LSR_REG] & UART_LSR_DR) == 0) {
        /* Busy Wait */
    }

    /* Zeichen aus Empfangsregister lesen */
    return base[UART_RHR_REG];
}

static int uart_tstc(void *priv) {
    volatile uint8_t *base = (volatile uint8_t *)priv;
    if (!base) {
        return 0;
    }

    /* Prüfen, ob Daten im Puffer liegen */
    return (base[UART_LSR_REG] & UART_LSR_DR) ? 1 : 0;
}

/* Globale Instanz der Operationen für diesen Treiber */
struct console_ops uart_console_ops = {
    .init = uart_init,
    .putc = uart_putc,
    .getc = uart_getc,
    .tstc = uart_tstc
};
