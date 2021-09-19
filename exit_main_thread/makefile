CC=gcc
CFLAGS=-Wall -Wextra -pthread -g3
OFLAGS=-O2 -march=native -mtune=native

.PHONY: all clean

all: exit_main_thread.exe shared_counter.exe producer_consumer.exe

exit_main_thread.exe: exit_main_thread.c
	$(CC) $(CFLAGS) $(OFLAGS) $< -o $@

# tell the compiler not to delete the while loop whose counter is waiting for
# a given number by passing -O0 optimization flag
shared_counter.exe: shared_counter.c
	$(CC) $(CFLAGS) -O0 $< -o $@

producer_consumer.exe: producer_consumer.c
	$(CC) $(CFLAGS) $(OFLAGS) $< -o $@

clean:
	rm -Rf *~ *.exe
