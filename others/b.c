080483ed <dtof>:
 80483ed:	55                   	push   %ebp
 80483ee:	89 e5                	mov    %esp,%ebp
 80483f0:	83 ec 0c             	sub    $0xc,%esp
 80483f3:	8b 45 08             	mov    0x8(%ebp),%eax
 80483f6:	89 45 f8             	mov    %eax,-0x8(%ebp)
 80483f9:	8b 45 0c             	mov    0xc(%ebp),%eax
 80483fc:	89 45 fc             	mov    %eax,-0x4(%ebp)
 80483ff:	dd 45 f8             	fldl   -0x8(%ebp)
 8048402:	d9 5d f4             	fstps  -0xc(%ebp)
 8048405:	d9 45 f4             	flds   -0xc(%ebp)
 8048408:	c9                   	leave  
 8048409:	c3                   	ret    


float dtof(double x)
{
	return (float)x;
	asm volatile (
		"fldl -0x8(%ebp)\n"
		"fstps -0xc(%ebp)\n"
		"flds -0xc(%ebp)\n"
		);
}

int main()
{
	double a = 423.33;
	float b = dtof(a);
	return 0;
}