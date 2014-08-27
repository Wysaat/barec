#include <stdio.h>

typedef struct {
	int a;
	int b;
	int c;
} info;

struct tom {
	int a;
	int b;
	int c;
	int d;
};

info f()
{
	info retval = { 3, 4, 5 };
	return retval;
}

int g(int x)
{
	char x = 43;
	if (x > 3)
		return 3;
	else
		puts("hello");
}

struct tom h()
{
	struct tom a = { 432, 432 , 4324, 55555 };
	return a;
}

int main()
{
	info val;
	printf("val.a is %d\n", val.a);

	return 0;
}