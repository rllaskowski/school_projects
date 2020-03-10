CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS =

.PHONY: all clean

all: main

main: main.o avl.o
	$(CC) $(LDFLAGS) -o $@ $^

avl.o: avl.c avl.h
	$(CC) $(CFLAGS) -c $<

main.o: main.c avl.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o main
