CC = gcc
CFLAGS = -Wall

.PHONY: all clean

all: testhttp_raw

testhttp_raw: testhttp_raw.o
	$(CC) $(CFLAGS) -o $@ $<

testhttp_raw.o: testhttp_raw.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o testhttp_raw
