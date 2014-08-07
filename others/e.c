080483ed <itold>:
 80483ed:	55                   	push   %ebp
 80483ee:	89 e5                	mov    %esp,%ebp
 80483f0:	db 45 08             	fildl  0x8(%ebp)
 80483f3:	5d                   	pop    %ebp
 80483f4:	c3                   	ret    

080483f5 <itod>:
 80483f5:	55                   	push   %ebp
 80483f6:	89 e5                	mov    %esp,%ebp
 80483f8:	db 45 08             	fildl  0x8(%ebp)
 80483fb:	5d                   	pop    %ebp
 80483fc:	c3                   	ret    

080483fd <itof>:
 80483fd:	55                   	push   %ebp
 80483fe:	89 e5                	mov    %esp,%ebp
 8048400:	db 45 08             	fildl  0x8(%ebp)
 8048403:	5d                   	pop    %ebp
 8048404:	c3                   	ret    

long double itold(int x) {
	return (long double)x;
}

double itod(int x) {
	return (double)x;
}

float itof(int x) {
	return (float)x;
}

int main()
{
	int x = 1024;
	long double a = itold(x);
	double b = itod(x);
	float c = itof(x);
	return 0;
}