FILENAME = dcl.asm
OUTPUT_FILE = dcl

.PHONY: all clean

all: main

main:
	rm -f *.o ${OUTPUT_FILE}
	nasm -f elf64 -w+all -w+error -o dcl.o dcl.asm
	ld --fatal-warnings -o dcl dcl.o

clean:
	rm -f *.o ${OUTPUT_FILE}
