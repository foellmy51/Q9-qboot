CC = gcc
CFLAGS = -Wall -Wextra -O2

all: host_simulator

host_simulator: main.o memory_backend.o
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ main.c

memory_backend.o: memory_backend.c
	$(CC) $(CFLAGS) -c -o $@ memory_backend.c

clean:
	rm -f *.o host_simulator
