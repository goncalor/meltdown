#include <stdio.h>
#include <signal.h>
#include <x86intrin.h>
#include <setjmp.h>

static jmp_buf context;

void segfault_handler() {
	puts("received SIGSEGV");
	longjmp(context, 1);
}

int main() {

	unsigned char array[4096*256];

	// setup segfault handling
	struct sigaction segfault_act;
	segfault_act.sa_handler = segfault_handler;
	sigaction(SIGSEGV, &segfault_act, NULL);

	asm("" : : "b"(array));

	if(setjmp(context))
		goto end;

	asm("mov $0x10, %ecx");
	asm("mov (%ecx), %eax");
	asm("shl $0xc, %eax");
	asm("mov (%ebx,%eax), %ebx");

end:
	return 0;
}
