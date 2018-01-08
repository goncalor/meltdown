#include <stdio.h>
#include <signal.h>
#include <x86intrin.h>
#include <setjmp.h>

#define PAGE_SIZE (4096)

int main() {

	unsigned char array[PAGE_SIZE*256];
	unsigned int cycles_high_start, cycles_low_start,
				 cycles_high_end, cycles_low_end;
	unsigned long cycles;
	int i;

	for(i=0; i<256; i++)
		_mm_clflush(&array[PAGE_SIZE*i]);

	array[PAGE_SIZE*15] = 123;

	// // https://stackoverflow.com/a/14214220/2057521
	asm volatile (
			"cpuid\n\t"/*serialize*/
			"rdtsc\n\t"/*read the clock*/
			"mov %%edx, %0\n\t"
			"mov %%eax, %1\n\t": "=r" (cycles_high_start), "=r"
			(cycles_low_start):: "%rax", "%rbx", "%rcx", "%rdx");

	asm volatile ("" : : "c"(array[PAGE_SIZE*15]));

	asm volatile (
			"rdtscp\n\t"/*read the clock*/
			"mov %%edx, %0\n\t"
			"mov %%eax, %1\n\t"
			"cpuid\n\t": "=r" (cycles_high_end), "=r"
			(cycles_low_end):: "%rax", "%rbx", "%rcx", "%rdx");

	cycles = (((unsigned long) cycles_high_end << 32) | cycles_low_end) -
		((unsigned long) cycles_high_start << 32 | cycles_low_start);

	printf("cycles %3d: %lu\n", i, cycles);

	return 0;
}
