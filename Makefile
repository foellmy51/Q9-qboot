CC = gcc
CFLAGS = -Wall -Wextra -Isrc/include -O2 -g

SRCS = src/boot/main.c        src/cli/cli.c        src/bios/bios_core.c        src/bios/console_uart.c        src/bios/console_fb.c        src/lib/lib_core.c

OBJS = $(SRCS:.c=.o)
TARGET = qboot

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
