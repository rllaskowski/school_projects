CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGETS = radio-proxy

all: $(TARGETS) 

utils.o: utils.c utils.h

err.o: err.c err.h

radio-proxy.o: radio-proxy.c utils.h

radio-proxy: radio-proxy.o utils.o err.o

clean:
	rm -f *.o $(TARGETS) 
