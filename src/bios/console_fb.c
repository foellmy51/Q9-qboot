#include "qboot_hal.h"

/**
 * @brief Zustandsinformationen für die Framebuffer-Konsole
 */
struct fb_info {
    uint32_t *vram;        /**< Zeiger auf den pixelbasierten Videospeicher (ARGB) */
    uint32_t width;       /**< Bildschirmbreite in Pixeln */
    uint32_t height;      /**< Bildschirmhöhe in Pixeln */
    uint32_t cursor_x;    /**< Aktuelle Text-Spalte (in Zeichen) */
    uint32_t cursor_y;    /**< Aktuelle Text-Zeile (in Zeichen) */
    uint32_t fg_color;    /**< Vordergrundfarbe (0xAARRGGBB) */
    uint32_t bg_color;    /**< Hintergrundfarbe (0xAARRGGBB) */
};

/* Ein einfaches, schematisches 8x8 Bitmap-Font-Array für Demonstrationszwecke.
 * In einem echten Treiber enthält dies Zeichendefinitionen für alle ASCII-Zeichen.
 */
static const uint8_t fb_font_8x8[256][8] = {
    ['A'] = { 0x18, 0x24, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00 },
    ['B'] = { 0x7C, 0x42, 0x42, 0x7C, 0x42, 0x42, 0x7C, 0x00 },
    ['C'] = { 0x3C, 0x42, 0x40, 0x40, 0x40, 0x42, 0x3C, 0x00 },
    /* ... weitere Zeichen */
};

static int fb_init(void *priv) {
    struct fb_info *fb = (struct fb_info *)priv;
    if (!fb || !fb->vram) {
        return -1;
    }

    fb->cursor_x = 0;
    fb->cursor_y = 0;
    fb->fg_color = 0xFFFFFFFF; /* Weiß */
    fb->bg_color = 0xFF000000; /* Schwarz (undurchsichtig) */

    /* Bildschirm mit Hintergrundfarbe leeren */
    for (uint32_t i = 0; i < fb->width * fb->height; i++) {
        fb->vram[i] = fb->bg_color;
    }

    return 0;
}

static void fb_draw_char(struct fb_info *fb, char c, uint32_t cx, uint32_t cy) {
    uint32_t screen_x = cx * 8;
    uint32_t screen_y = cy * 8;
    const uint8_t *char_bitmap = fb_font_8x8[(uint8_t)c];

    for (int y = 0; y < 8; y++) {
        uint8_t row = char_bitmap[y];
        for (int x = 0; x < 8; x++) {
            /* Prüfen, ob das Bit an Position (7-x) gesetzt ist */
            uint32_t color = (row & (1 << (7 - x))) ? fb->fg_color : fb->bg_color;
            
            uint32_t px = screen_x + x;
            uint32_t py = screen_y + y;

            if (px < fb->width && py < fb->height) {
                fb->vram[py * fb->width + px] = color;
            }
        }
    }
}

static void fb_putc(void *priv, char c) {
    struct fb_info *fb = (struct fb_info *)priv;
    if (!fb) {
        return;
    }

    if (c == '\n') {
        fb->cursor_x = 0;
        fb->cursor_y++;
    } else if (c == '\r') {
        fb->cursor_x = 0;
    } else {
        fb_draw_char(fb, c, fb->cursor_x, fb->cursor_y);
        fb->cursor_x++;
        
        /* Automatische Zeilenumbruch bei Erreichen des rechten Rands */
        if (fb->cursor_x * 8 >= fb->width) {
            fb->cursor_x = 0;
            fb->cursor_y++;
        }
    }

    /* Scroll-Prüfung */
    if (fb->cursor_y * 8 >= fb->height) {
        /* In einer vollen Implementierung kopiert man den Bildschirm um 8 Zeilen nach oben:
         * memmove(fb->vram, fb->vram + (fb->width * 8), fb->width * (fb->height - 8) * sizeof(uint32_t));
         * Und leert die untersten 8 Zeilen.
         * Für diese Vorschau setzen wir den Cursor einfach zurück:
         */
        fb->cursor_y = 0;
    }
}

static char fb_getc(void *priv) {
    /* Ein Framebuffer liest keine Tastatureingaben.
     * Keyboard-Input wird typischerweise über einen PS/2- oder USB-Treiber realisiert.
     */
    (void)priv;
    return 0;
}

static int fb_tstc(void *priv) {
    (void)priv;
    return 0;
}

/* Instanz für den Framebuffer */
struct console_ops fb_console_ops = {
    .init = fb_init,
    .putc = fb_putc,
    .getc = fb_getc,
    .tstc = fb_tstc
};
