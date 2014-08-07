#include <stdio.h>

080483ed <fgt>:
 80483ed:	55                   	push   %ebp
 80483ee:	89 e5                	mov    %esp,%ebp
 80483f0:	d9 45 08             	flds   0x8(%ebp)
 80483f3:	d9 45 0c             	flds   0xc(%ebp)
 80483f6:	d9 c9                	fxch   %st(1)
 80483f8:	df e9                	fucomip %st(1),%st  /* FCOMI/FCOMIP/ FUCOMI/FUCOMIP—Compare Floating Point Values and Set EFLAGS */
 80483fa:	dd d8                	fstp   %st(0)
 80483fc:	0f 97 c0             	seta   %al
 80483ff:	0f b6 c0             	movzbl %al,%eax /* intel: movzx  eax,al */
 8048402:	5d                   	pop    %ebp
 8048403:	c3                   	ret

int fgt(float a, float b) {
	return a > b;
}

int main()
{
	float a = 4324.324;
	float b = 322.2;
	int x = fgt(a, b);
	return 0;
}