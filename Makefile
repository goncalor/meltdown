all:
	gcc -Wall -O0 meltdown.c -o meltdown
	gcc -Wall -O0 -c meltdown.c
	objdump -d meltdown.o
