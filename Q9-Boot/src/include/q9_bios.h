#ifndef Q9_BIOS_H
#define Q9_BIOS_H

#include <stdint.h>
#include <stddef.h>
#include "q9_descriptors.h"

/* BIOS Fehlercodes */
#define Q9_BIOS_SUCCESS          0
#define Q9_BIOS_ERR_GENERIC     -1
#define Q9_BIOS_ERR_INVALID_VAL -2
#define Q9_BIOS_ERR_TIMEOUT     -3
#define Q9_BIOS_ERR_IO          -4
#define Q9_BIOS_ERR_NO_MEDIA    -5
#define Q9_BIOS_ERR_NOT_READY   -6

/**
 * @brief BIOS Vektortabelle (Vector Table)
 * Diese Struktur liegt an einer festen, dokumentierten Adresse im Speicherraum
 * (z.B. am Anfang des ROMs oder wird über ein Syscall-Trap zur Verfügung gestellt).
 * Dadurch kann Q9-OS die BIOS-Dienste ohne feste Link-Adressen aufrufen.
 */
struct q9_bios_vectors {
    /* Versions- und System-Infos */
    uint32_t bios_version;   /**< Versionsnummer, z.B. 0x00010000 (v1.0.0) */
    uint32_t qumips_rating;  /**< Berechnete Systemleistung (MIPS) */

    /* --- System- & Deskriptor-Dienste --- */
    /**
     * @brief Liefert die Liste aller erkannten Gerätedeskriptoren.
     * @param dest_array Puffer, in den die Deskriptoren kopiert werden.
     * @param max_count Maximale Anzahl der zu lesenden Deskriptoren.
     * @return Anzahl der kopierten Deskriptoren.
     */
    int (*get_descriptors)(struct q9_dev_header *dest_array, int max_count);

    /* --- Konsolen-Dienste --- */
    /**
     * @brief Initialisiert eine Konsole basierend auf ihrem SCF-Deskriptor.
     */
    int (*console_init)(struct q9_scf_descriptor *desc);
    
    /**
     * @brief Gibt ein Zeichen auf der aktiven Konsole aus.
     */
    void (*console_putc)(char c);
    
    /**
     * @brief Wartet auf ein Zeichen der Konsole (blockierend).
     */
    char (*console_getc)(void);
    
    /**
     * @brief Prüft, ob ein Zeichen im Eingangspuffer liegt (nicht blockierend).
     * @return 1 wenn bereit, 0 wenn leer.
     */
    int (*console_tstc)(void);

    /* --- Massenspeicher-Dienste (RBF) --- */
    /**
     * @brief Initialisiert ein Laufwerk basierend auf seinem RBF-Deskriptor.
     */
    int (*storage_init)(struct q9_rbf_descriptor *desc);
    
    /**
     * @brief Fragt detaillierte Informationen über das Laufwerk ab.
     */
    int (*storage_info)(struct q9_rbf_descriptor *desc);

    /**
     * @brief Liest Sektoren von einem Massenspeicher.
     * @param desc Der RBF-Deskriptor des Ziellaufwerks.
     * @param sector Startsektor (LBA).
     * @param count Anzahl der zu lesenden Sektoren.
     * @param buffer Zielpuffer im RAM.
     * @return Q9_BIOS_SUCCESS oder Fehlercode.
     */
    int (*storage_read)(struct q9_rbf_descriptor *desc, uint32_t sector, uint32_t count, void *buffer);

    /**
     * @brief Schreibt Sektoren auf einen Massenspeicher.
     * @param desc Der RBF-Deskriptor des Ziellaufwerks.
     * @param sector Startsektor (LBA).
     * @param count Anzahl der zu schreibenden Sektoren.
     * @param buffer Quellpuffer im RAM.
     * @return Q9_BIOS_SUCCESS oder Fehlercode.
     */
    int (*storage_write)(struct q9_rbf_descriptor *desc, uint32_t sector, uint32_t count, const void *buffer);

    /* --- Systemsteuerung --- */
    /**
     * @brief Führt einen System-Reset (Soft- oder Hard-Reset) aus.
     */
    void (*system_reset)(void);
};

/* Globale Instanz der Vektortabelle im BIOS-Sicherheitsbereich */
extern const struct q9_bios_vectors bios_vector_table;

#endif /* Q9_BIOS_H */
