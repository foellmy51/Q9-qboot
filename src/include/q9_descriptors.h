#ifndef Q9_DESCRIPTORS_H
#define Q9_DESCRIPTORS_H

#include <stdint.h>

/* Device Klassen analog zu OS-9 */
#define Q9_DEV_CLASS_SCF  1  /* Sequential Character File (UART, Terminal, Tastatur, Framebuffer) */
#define Q9_DEV_CLASS_RBF  2  /* Random Block File (SD-Karte, Flash, Festplatte) */
#define Q9_DEV_CLASS_NFM  3  /* Network File Manager (Netzwerkschnittstellen) */

/* Hardware Typen */
#define Q9_HW_TYPE_UART_16550   1
#define Q9_HW_TYPE_FRAMEBUFFER  2
#define Q9_HW_TYPE_SD_CARD      10
#define Q9_HW_TYPE_SPI_FLASH    11
#define Q9_HW_TYPE_IDE          12
#define Q9_HW_TYPE_ETHERNET_NIC 20

/**
 * @brief Gemeinsamer Header für alle Q9 Gerätedeskriptoren
 */
struct q9_dev_header {
    uint16_t class_id;       /**< Q9_DEV_CLASS_* */
    uint16_t hw_type;        /**< Q9_HW_TYPE_* */
    char name[16];           /**< Eindeutiger Name des Geräts (z.B. "/term0", "/h0") */
    uint32_t port_address;   /**< Physikalische Basisadresse der Register im Speicherraum */
    uint8_t irq_vector;      /**< Interrupt-Vektor für dieses Gerät */
    uint8_t irq_priority;    /**< Priorität des Interrupts */
    uint16_t reserved;
};

/**
 * @brief Deskriptor für serielle oder zeichenbasierte Geräte (SCF)
 */
struct q9_scf_descriptor {
    struct q9_dev_header header;
    
    /* UART Spezifisch */
    uint32_t baudrate;       /**< z.B. 115200 */
    uint8_t parity;          /**< 0 = Keine, 1 = Odd, 2 = Even */
    uint8_t stop_bits;       /**< 1 oder 2 */
    uint8_t data_bits;       /**< 5, 6, 7 oder 8 */
    uint8_t flow_control;    /**< 0 = Keine, 1 = RTS/CTS, 2 = XON/XOFF */

    /* Grafische Konsole Spezifisch (falls hw_type == Q9_HW_TYPE_FRAMEBUFFER) */
    uint16_t fb_width;       /**< z.B. 800 */
    uint16_t fb_height;      /**< z.B. 600 */
    uint8_t fb_bpp;          /**< Bits pro Pixel (z.B. 16, 32) */
    uint8_t reserved2[3];
};

/**
 * @brief Deskriptor für blockbasierte Speichergeräte (RBF)
 */
struct q9_rbf_descriptor {
    struct q9_dev_header header;
    
    uint32_t block_size;     /**< Sektorgröße in Bytes (meistens 512) */
    uint32_t total_blocks;   /**< Gesamtzahl der Blöcke auf dem Medium */
    
    /* Dateisystem und Partitionsinformationen */
    uint8_t partition_type;  /**< z.B. 0x0C für FAT32 (LBA), 0x83 für Linux Native */
    uint8_t fs_format;       /**< Erkanntes Format: 0 = Unbekannt, 1 = FAT16, 2 = FAT32, 3 = ext2, 4 = Raw */
    uint8_t is_bootable;     /**< 1 = Enthält startfähiges OS, 0 = nur Daten */
    uint8_t reserved[5];
};

/**
 * @brief Deskriptor für Netzwerkschnittstellen (NFM)
 */
struct q9_nfm_descriptor {
    struct q9_dev_header header;
    
    uint8_t mac_address[6];  /**< MAC-Adresse des Netzwerkgeräts */
    uint8_t ip_address[4];   /**< Statisch konfigurierte IP (oder 0.0.0.0 für DHCP) */
    uint8_t reserved[6];
};

#endif /* Q9_DESCRIPTORS_H */
