#include <stdio.h>

int main()
{
	int a[3] = { 1, 2, 3 };
	printf("(&a)[2] is %d, or %p\n", (&a)[2], (&a)[2]);
	printf("a[2] is %d\n", a[2]);
	printf("a is %p, &a is %p\n", a, &a);
	printf("sizeof(int) is %d\n", sizeof(int));
	printf("(&a)[0] is %p, (&a)[1] is %p, (&a)[2] is %p\n", (&a)[0], (&a)[1], (&a)[2]);
	printf("sizeof(&a) is %d, sizeof(a) is %d, sizeof(*a) is %d\n", sizeof(&a), sizeof(a), sizeof(*a));

	printf("a is %p, a+1 is %p, a+2 is %p\n", a, a+1, a+2);
	printf("&a is %p, &a+1 is %p, &a+2 is %p\n", &a, &a+1, &a+2);

	return 0;
}