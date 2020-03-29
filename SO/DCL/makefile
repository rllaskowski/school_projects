FILENAME = dcl.asm
OUTPUT_FILE = dcl

.PHONY: all clean

all: main

main:
	nasm -f elf64 -w+all -w+error -o dcl.o dcl.asm
	ld --fatal-warnings -o dcl dcl.o
debug:
	yasm -g dwarf2 -f elf64 dcl.asm -l dcl.lst
	ld -g -o dcl dcl.o 
clean:
	rm -f *.o ${OUTPUT_FILE} dcl.lst
