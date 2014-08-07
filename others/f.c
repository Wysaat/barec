 80483ed:	55                   	push   %ebp
 80483ee:	89 e5                	mov    %esp,%ebp
 80483f0:	83 7d 08 00          	cmpl   $0x0,0x8(%ebp)
 80483f4:	74 0d                	je     8048403 <and+0x16>
 80483f6:	83 7d 0c 00          	cmpl   $0x0,0xc(%ebp)
 80483fa:	74 07                	je     8048403 <and+0x16>
 80483fc:	b8 01 00 00 00       	mov    $0x1,%eax
 8048401:	eb 05                	jmp    8048408 <and+0x1b>
 8048403:	b8 00 00 00 00       	mov    $0x0,%eax
 8048408:	5d                   	pop    %ebp
 8048409:	c3                   	ret

int and(int a, int b) {
	return a && b;
}

int main()
{
	int a = 3;
	int b = 4;
	int c = and(a, b);
	return 0;
}