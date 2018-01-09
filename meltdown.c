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
	unsigned long cycles_min = ~0;
	int i;
	int i_min = -1;
	unsigned char dummy;

	for(i=0; i<256; i++)
		array[PAGE_SIZE*i] = i;

	for(i=0; i<256; i++)
		_mm_clflush(&array[PAGE_SIZE*i]);

	asm volatile ("cpuid\n\t" ::: "%rax", "%rbx", "%rcx", "%rdx");

	dummy = array[PAGE_SIZE*15];
	printf("dummy %d\n", (unsigned int) dummy);

	for(i=0; i<256; i++) {
		// // https://stackoverflow.com/a/14214220/2057521
		asm volatile (
				"cpuid\n\t"/*serialize*/
				"rdtsc\n\t"/*read the clock*/
				"mov %%edx, %0\n\t"
				"mov %%eax, %1\n\t": "=r" (cycles_high_start), "=r"
				(cycles_low_start):: "%rax", "%rbx", "%rcx", "%rdx");

		dummy = array[PAGE_SIZE*i];

		asm volatile (
				"rdtscp\n\t"/*read the clock*/
				"mov %%edx, %0\n\t"
				"mov %%eax, %1\n\t"
				"cpuid\n\t": "=r" (cycles_high_end), "=r"
				(cycles_low_end):: "%rax", "%rbx", "%rcx", "%rdx");

		cycles = (((unsigned long) cycles_high_end << 32) | cycles_low_end) -
			((unsigned long) cycles_high_start << 32 | cycles_low_start);

		// printf("cycles %3d: %lu\n", i, cycles);

		if(cycles <= cycles_min) {
			cycles_min = cycles;
			i_min = i;
		}

	}

	printf("cycles %3d: %lu\n", i_min, cycles_min);
	printf("dummy %d\n", (unsigned int) dummy);

	return 0;
}
