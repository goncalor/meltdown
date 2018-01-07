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
	unsigned int cycles_high_start, cycles_low_start,
				 cycles_high_end, cycles_low_end;
	unsigned long cycles;

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
	// puts("receiving end");
	_mm_clflush(&array);

	// https://stackoverflow.com/a/14214220/2057521
	asm volatile (
			"cpuid\n\t"/*serialize*/
			"rdtsc\n\t"/*read the clock*/
			"mov %%edx, %0\n\t"
			"mov %%eax, %1\n\t": "=r" (cycles_high_start), "=r"
			(cycles_low_start):: "%rax", "%rbx", "%rcx", "%rdx");

	asm("" : : "c"(array[0]));

	asm volatile (
			"rdtscp\n\t"/*read the clock*/
			"mov %%edx, %0\n\t"
			"mov %%eax, %1\n\t"
			"cpuid\n\t": "=r" (cycles_high_end), "=r"
			(cycles_low_end):: "%rax", "%rbx", "%rcx", "%rdx");

	cycles = (((unsigned long) cycles_high_end << 32) | cycles_low_end) -
		((unsigned long) cycles_high_start << 32 | cycles_low_start);

	printf("cycles: %lu\n", cycles);

	return 0;
}
