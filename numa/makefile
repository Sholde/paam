CC=gcc
CFLAGS=-Wall -std=c99
OFLAGS=-O2 -march=native -march=native
DFLAGS=-g3
LFLAGS=-pthread -lhwloc

TARGET=main

.PHONY: all clean

all: $(TARGET)

$(TARGET): main.o
	$(CC) $(LFLAGS) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $(OFLAGS) $(DFLAGS) $< -o $@

main.o:

clean:
	rm -Rf *~ *.o $(TARGET)
