#include <stdio.h>

struct node {
	int a;
	int b;
	int c;
};

struct node f() {
	struct node a;
	a.a = 3;
	a.b = 4;
	a.c = 5;
	printf("in f,    &a is %p, a.a is %d\n", &a, a.a);
	return a;
}

int main()
{
	struct node a = f();
	printf("in main, &a is %p, a.a is %d\n", &a, a.a);
	return 0;
}