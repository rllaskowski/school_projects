CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -O2

.PHONY: all clean

all: forests

forests: main.o avl.o
	$(CC) $(LDFLAGS) -o $@ $^

avl.o: avl.c avl.h
	$(CC) $(CFLAGS) -c $<

main.o: main.c avl.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o forests
