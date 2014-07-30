#include <stdio.h>
#include <stdlib.h>

int main()
{
	int a = 32;
	int b = 5;
	static struct x {
		int m[- - 3];
		int n[5*7];
	} s;
	printf("sizeof(x) is %lu\n", sizeof(struct x));
	a = 3;
	printf("sizeof(x) is %lu\n", sizeof(struct x));
	return 0;
}
