#include "qboot_hal.h"

#ifndef BARE_METAL
/* --- Desktop Simulator-Modus --- */
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

static int uart_init(void *priv) {
    (void)priv;
    /* In der Simulation ist stdout/stdin bereits initialisiert */
    return 0;
}

static void uart_putc(void *priv, char c) {
    (void)priv;
    putchar(c);
    fflush(stdout);
}

static char uart_getc(void *priv) {
    (void)priv;
    return getchar();
}

static int uart_tstc(void *priv) {
    (void)priv;
    /* Nutzt select(), um im Terminal nicht-blockierend zu prüfen, ob eine Taste gedrückt wurde */
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0 ? 1 : 0;
}

#else
/* --- Physischer Bare-Metal-Modus --- */

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
    /* Hardware-Initialisierung (Baudrate etc.) */
    return 0;
}

static void uart_putc(void *priv, char c) {
    volatile uint8_t *base = (volatile uint8_t *)priv;
    if (!base) {
        return;
    }
    while ((base[UART_LSR_REG] & UART_LSR_THRE) == 0) {
        /* Busy Wait */
    }
    base[UART_THR_REG] = c;
}

static char uart_getc(void *priv) {
    volatile uint8_t *base = (volatile uint8_t *)priv;
    if (!base) {
        return 0;
    }
    while ((base[UART_LSR_REG] & UART_LSR_DR) == 0) {
        /* Busy Wait */
    }
    return base[UART_RHR_REG];
}

static int uart_tstc(void *priv) {
    volatile uint8_t *base = (volatile uint8_t *)priv;
    if (!base) {
        return 0;
    }
    return (base[UART_LSR_REG] & UART_LSR_DR) ? 1 : 0;
}
#endif

/* Globale Instanz der Operationen für diesen Treiber */
struct console_ops uart_console_ops = {
    .init = uart_init,
    .putc = uart_putc,
    .getc = uart_getc,
    .tstc = uart_tstc
};
