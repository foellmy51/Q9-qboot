|-------------------------------------------------------------------------
| File:         src/boot/def_startup.S
| Desc:         Stage 1 Assembly Bootstrap (Default/Generisches Modul)
|               Fuer standardmaessige 68k System-Boards ohne ROM-Overlay.
|-------------------------------------------------------------------------

.global _start

| Definitionen für ein standardmäßiges System
RAM_BASE          = $00000000     | Standardmäßiger RAM-Anfang
RAM_TOP           = $00FFFFFF     | Standardmäßiges RAM-Ende (z.B. 16 MB)

.section .vectors
| Reset-Vektoren
    .long   RAM_TOP               | 0x00000000: Stack Pointer (SP)
    .long   _start                | 0x00000004: Program Counter (PC)

.section .text
_start:
    | --- SCHRITT 1: CPU-Status initialisieren ---
    ori.w   #$2700, sr            | Interrupts sperren, Supervisor-Modus

    | --- SCHRITT 2: Vektor-Basis-Register (VBR) initialisieren ---
    | Auf einem Standard 68010+ Board initialisieren wir das VBR auf RAM-Anfang.
    move.l  #RAM_BASE, d0
    .word   $4e7b, $0801          | movec d0, vbr

    | --- SCHRITT 3: C-Stack aufsetzen ---
    move.l  #RAM_TOP, sp

    | --- SCHRITT 4: Sprung ins C-Hauptprogramm ---
    jsr     main

    | Endlosschleife falls main() zurückkehrt
.halt_system:
    stop    #$2700
    bra     .halt_system
