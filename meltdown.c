#include <stdio.h>
#include <signal.h>
#include <x86intrin.h>

void segfault_handler() {

	puts("received SIGSEGV");
}

int main() {

	unsigned char array[4096*256];

	// setup segfault handling
	struct sigaction segfault_act;
	segfault_act.sa_handler = segfault_handler;
	sigaction(SIGSEGV, &segfault_act, NULL);

	asm("" : : "b"(array));

	asm("mov $0x10, %ecx");
	asm("mov (%ecx), %eax");
	asm("shl $0xc, %eax");
	asm("mov (%ebx,%eax), %ebx");

	return 0;
}
