#include <stdio.h>

/* 
 * SARX keeps and propagates the most significant bit (sign bit) while shifting.
 */

/*
 * the FPU ergisters include eight 80-bit data registers, and three 16-bit reigisters
 * called the control, status and tag registers.
 * the FPU data registers are called R0 throught R7 (but they are not accessed by these
 * names), the FPU register stack is circular -- that is, the last register in the stack
 * links back to the first register in the stack.
 * the register that is considered the top of the stack is defined in the FPU control
 * word register. it s referenced by the name ST(0). each of hte other registers is
 * referenced relative to the top register, by the name ST(x), where x can be 1 through
 * 7.
 *
 * as data is loaded into the FPU stack, the stack top moves downward in the registers.
 */

double a = 123.45;
double b = 394342.332;
double res;

unsigned m = 432;
unsigned n = 93;
unsigned long long t;

int x = -9433;
int y = 10249;
long long  z;

unsigned long long aa = 23443243253252342;
unsigned long long bb = 43242423;
unsigned long long cc;

long long xx = -43242593242593024;
long long yy = 39320490151;
long long zz;

/*
 * at&t syntax:
 *   q for 64-bit
 *   l for 32-bit
 *   w for 16-bit
 *   b for 8-bit
 */

/*
 * CVTTSS2SI - Convert with Truncation Scalar Single-Precision FP Value to Dword Integer
 * e.g. cvttss2si %xmm0,%eax
 */

int main()
{

	/* float * float */
	__asm__ __volatile__(
		"fldl a\n\
		 fldl b\n\
		 fmul %%st(1), %%st(0)\n\
		 fstl res\n\
		 " : : );
	printf("a is %f, b is %f, res is %f\n", a, b, res);

	/* unsigned * unsigned */
	__asm__ __volatile__(
		"push n\n\
		 mov m, %%eax\n\
		 mull (%%esp)\n\
		 mov %%eax, t\n\
		 mov %%edx, t+4\n\
		 pop %%ebx\n\
		 " : : );
	printf("m is %u, n is %u, t is %Lu\n", m, n, t);

	/* int * int */
	__asm__ __volatile__(
		"push y\n\
		 mov x, %%eax\n\
		 imull (%%esp)\n\
		 mov %%eax, z\n\
		 mov %%edx, z+4\n\
		 pop %%ebx\n\
		 " : : );
	printf("x is %d, y is %d, z is %Ld\n", x, y, z);

	/* unsigned long long * unsigned long long */
	__asm__ __volatile__(
		/*
		 * ((aa_hi * bb_lo) << 32) +
		 * ((aa_lo * bb_hi) << 32) +
		 * ((aa_lo * bb_lo))
		 */
		"push bb\n\
		 push bb+4\n\
         \n\
         pop %%ebx\n\
         pop %%ecx\n\
         \n\
		 mov aa+4, %%eax\n\
		 mull %%ecx\n\
		 push %%eax\n\
         \n\
		 mov aa, %%eax\n\
		 mull %%ebx\n\
		 add %%eax, (%%esp)\n\
		 \n\
		 mov aa, %%eax\n\
		 mull %%ecx\n\
		 pop %%ebx\n\
		 add %%ebx, %%edx\n\
		 \n\
		 mov %%eax, cc\n\
		 mov %%edx, cc+4\n\
		 " : : );
	printf("aa is %Lu, bb is %Lu, cc is %Lu\n", aa, bb, cc);

	/* long long * long long */
	__asm__ __volatile__(
		"push bb\n\
		 push bb+4\n\
		 \n\
		 pop %%ebx\n\
		 pop %%ecx\n\
		 \n\
		 and %%ebx, 0x10000000\n\
		" : : );

	return 0;
}