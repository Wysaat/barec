#include <stdio.h>
#include <stdlib.h>

typedef struct list {
    int value;
} list;

typedef struct dummy {
    list *list;
} dummy;

int func() {
	auto int a = 3, b;
	a = b * 3;
}

int main()
{
	int **a;
	int b = a;
	int c = &a;
	printf("b is %d\n", b);
	printf("c is %d\n", c);
	return 0;
}