// 64-bit 00000000004004ed <ldmul>:
//   4004ed:	55                   	push   %rbp
//   4004ee:	48 89 e5             	mov    %rsp,%rbp
//   4004f1:	db 6d 10             	fldt   0x10(%rbp)
//   4004f4:	db 6d 20             	fldt   0x20(%rbp)
//   4004f7:	de c9                	fmulp  %st,%st(1)
//   4004f9:	5d                   	pop    %rbp
//   4004fa:	c3                   	retq

// 32-bit 080483ed <ldmul>:
//  80483ed:	55                   	push   %ebp
//  80483ee:	89 e5                	mov    %esp,%ebp
//  80483f0:	db 6d 08             	fldt   0x8(%ebp)
//  80483f3:	db 6d 14             	fldt   0x14(%ebp)
//  80483f6:	de c9                	fmulp  %st,%st(1)
//  80483f8:	5d                   	pop    %ebp
//  80483f9:	c3                   	ret

long double ldmul(long double x, long double y)
{
	return x * y;
	asm volatile (
		"fldt 0x8(%ebp)\n"
		"fldt 0x14(%ebp)\n"
		"fmulp %st, %st(1)\n"
		);
}

int main()
{
	long double a = 43242.32223, b = 3242.222, c;
	c = a * b;
	return 0;
}