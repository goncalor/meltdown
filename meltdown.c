
#include <x86intrin.h>

int main() {

	unsigned char array[4096*256];

	asm("" : : "b"(array));

	asm("mov $0x10, %ecx");
	asm("mov (%ecx), %eax");
	asm("shl $0xc, %eax");
	asm("mov (%ebx,%eax), %ebx");

	return 0;
}
