# Makefile
LDFLAGS=-lncurses

a.out: dna.c
	gcc $(LDFLAGS) dna.c

