#include <stdio.h>

int static a = 3;

struct info {
	int a;
	int b;
};

static int f(register int x) {
	static char m[] = "hello";
	static int t = {33};
	static int a = 0;
	printf("m is %p, &t is %p, &a is %p\n", m, &t, &a);
	return 3;
}

int main()
{
	int x = 3;
	int y = x + x;
	printf("y is %d\n", y);
	printf("&a is %p\n", &a);
	f(3);
	return 0;
}