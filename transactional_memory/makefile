CC=gcc
CFLAGS=-Wall
OFLAGS=-O2 -march=native -mtune=native
LFLAGS=-pthread -latomic

TARGET=main.exe

.PHONY: all clean

all: $(TARGET)

main.exe: main.c
	$(CC) $(CFLAGS) $(OFLAGS) $(LFLAGS) $< -o $@

clean:
	rm -Rf *~ $(TARGET)
