CC=gcc
CFLAGS=-Wall -pthread -latomic
OFLAGS=-O2 -march=native -mtune=native

.PHONY: all clean

all: mutex_queue.exe lock_free_queue.exe

%.exe: %.c
	$(CC) $(CFLAGS) $(OFLAGS) $< -o $@

clean:
	rm -Rf *~ *.exe
