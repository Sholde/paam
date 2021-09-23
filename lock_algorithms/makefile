CC=gcc
CFLAGS=-Wall -pthread
OFLAGS=-O2 -march=native -mtune=native

.PHONY: all clean

all: benchmark.exe

%.exe: %.c
	$(CC) $(CFLAGS) $(OFLAGS) $< -o $@

clean:
	rm -Rf *~ *.exe
