CC ?= gcc
CFLAGS ?= -Wall -Wextra -Isrc/include -O2 -g

# C-Quelldateien für Simulator und Baremetal
SRCS = src/boot/main.c \
       src/bios/q9_bogomips.c \
       src/cli/cli.c \
       src/bios/bios_core.c \
       src/bios/console_uart.c \
       src/bios/console_fb.c \
       src/lib/lib_core.c

OBJS = $(SRCS:.c=.o)
TARGET = qboot

# Standard-Ziel: Kompiliert den interaktiven Desktop-Simulator auf dem Mac
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) src/boot/*.o

# --- Bare-Metal Cross-Compilation Target für m68k ---
# Nutzung: make baremetal CROSS_COMPILE=m68k-elf- BOARD=cb030
CROSS_COMPILE ?= m68k-elf-
BOARD ?= cb030

BAREMETAL_CC = $(CROSS_COMPILE)gcc
BAREMETAL_AS = $(CROSS_COMPILE)as
BAREMETAL_CFLAGS = -Wall -Wextra -Isrc/include -O2 -g -DBARE_METAL -m68030

BAREMETAL_ASM_SRCS = src/boot/$(BOARD)_startup.S
BAREMETAL_ASM_OBJS = $(BAREMETAL_ASM_SRCS:.S=.o)
BAREMETAL_OBJS = $(BAREMETAL_ASM_OBJS) $(OBJS)

%.o: %.S
	$(BAREMETAL_AS) -m68030 $< -o $@

baremetal: $(BAREMETAL_OBJS)
	$(BAREMETAL_CC) $(BAREMETAL_CFLAGS) -nostdlib -Ttext 0x00F00000 -o qboot_$(BOARD).elf $(BAREMETAL_OBJS)
	$(CROSS_COMPILE)objcopy -O binary qboot_$(BOARD).elf qboot_$(BOARD).bin
	@echo "Baremetal-ROM-Build abgeschlossen: qboot_$(BOARD).bin"

.PHONY: all clean baremetal
