080483ed <fmul>:
 80483ed:	55                   	push   %ebp
 80483ee:	89 e5                	mov    %esp,%ebp
 80483f0:	d9 45 08             	flds   0x8(%ebp)
 80483f3:	d8 4d 0c             	fmuls  0xc(%ebp)
 80483f6:	5d                   	pop    %ebp
 80483f7:	c3                   	ret    

080483f8 <dmul>:
 80483f8:	55                   	push   %ebp
 80483f9:	89 e5                	mov    %esp,%ebp
 80483fb:	83 ec 10             	sub    $0x10,%esp
 80483fe:	8b 45 08             	mov    0x8(%ebp),%eax
 8048401:	89 45 f8             	mov    %eax,-0x8(%ebp)
 8048404:	8b 45 0c             	mov    0xc(%ebp),%eax
 8048407:	89 45 fc             	mov    %eax,-0x4(%ebp)
 804840a:	8b 45 10             	mov    0x10(%ebp),%eax
 804840d:	89 45 f0             	mov    %eax,-0x10(%ebp)
 8048410:	8b 45 14             	mov    0x14(%ebp),%eax
 8048413:	89 45 f4             	mov    %eax,-0xc(%ebp)
 8048416:	dd 45 f8             	fldl   -0x8(%ebp)        ; intel: fld qword
 8048419:	dc 4d f0             	fmull  -0x10(%ebp)
 804841c:	c9                   	leave  
 804841d:	c3                   	ret    


float fmul(float x, float y) {
	return x * y;
	asm volatile (
		"flds 0x8(%ebp)\n"  (intel: fld dword [ebp+0x8])
		"fmuls %0xc(%ebp)\n"
		);
}

double dmul(double x, double y) {
	return x * y;
	asm volatile (
		"fldl 0x8(%ebp)\n"
		"fmull 0x10(%ebp)\n"
		);
}

int main()
{
	float a = 3.2;
	float b = 4.7;
	float c = fmul(a, b);
	double x = 3.2;
	double y = 4.7;
	double z = dmul(x, y);

	return 0;
}