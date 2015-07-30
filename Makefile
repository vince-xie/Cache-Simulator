CC=gcc
CFLAGS= -ansi -pedantic -Wall -m32

c-sim : c-sim.c
	$(CC) $(CFLAGS) $^ -o $@
clean:
	rm c-sim
