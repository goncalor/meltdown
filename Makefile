all:
	gcc -Wall meltdown.c
	gcc -Wall -c meltdown.c
	objdump -d meltdown.o
