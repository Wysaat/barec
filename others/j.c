#include <stdio.h>

int main()
{
	int x = 3;
	// auto int a = x;
	// // int a[x] = {1,2,3};
	// f(1,2,3);
	int y = {{x + x}};
	printf("y is %d\n", y);
	return 0;
}