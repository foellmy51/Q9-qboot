#ifndef QBOOT_HAL_H
#define QBOOT_HAL_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Konsolen-Schnittstelle (Console HAL)
 * Ermöglicht den nahtlosen Austausch zwischen UART, Framebuffer (Software-Rendering) oder Netzwerkkonsolen.
 */
struct console_ops {
    /**
     * @brief Initialisiert das Konsolengerät.
     * @param priv Zeiger auf private Hardware-Daten (z.B. Register-Basisadresse).
     * @return 0 bei Erfolg, Fehlercode andernfalls.
     */
    int (*init)(void *priv);

    /**
     * @brief Gibt ein einzelnes Zeichen auf der Konsole aus.
     */
    void (*putc)(void *priv, char c);

    /**
     * @brief Wartet auf ein Zeichen und liest es ein (blockierend).
     */
    char (*getc)(void *priv);

    /**
     * @brief Prüft, ob ein Zeichen im Empfangspuffer bereitliegt (nicht blockierend).
     * @return 1 wenn ein Zeichen bereitliegt, 0 wenn leer.
     */
    int (*tstc)(void *priv);
};

struct console_device {
    const char *name;
    struct console_ops *ops;
    void *priv;  /**< Hardware-spezifischer Zustand / Basisadresse */
};


/**
 * @brief Massenspeicher-Schnittstelle (Block Storage HAL)
 * Abstraktion für SD-Karten, IDE, SPI-Flash oder RAM-Disks.
 */
struct storage_ops {
    int (*init)(void *priv);
    int (*read_blocks)(void *priv, uint32_t start_lba, uint32_t count, void *buffer);
    int (*write_blocks)(void *priv, uint32_t start_lba, uint32_t count, const void *buffer);
};

struct storage_device {
    const char *name;
    uint32_t block_size;
    uint32_t total_blocks;
    struct storage_ops *ops;
    void *priv;
};


/**
 * @brief Netzwerkschnittstellen-Schnittstelle (Network HAL)
 * Abstraktion für Ethernet-Controller zur Unterstützung von BootP, DHCP und TFTP.
 */
struct net_ops {
    int (*init)(void *priv);
    int (*send)(void *priv, const void *packet, uint32_t length);
    int (*recv)(void *priv, void *buffer, uint32_t max_length);
    void (*get_mac)(void *priv, uint8_t mac[6]);
};

struct net_device {
    const char *name;
    struct net_ops *ops;
    void *priv;
};

#endif /* QBOOT_HAL_H */
