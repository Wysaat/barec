#include <stdio.h>

int main()
{
	char *a = "hello";
	char *b = "testing";
	char *c = "is funky";

	char *cent[] = { a, b, c };

	int (*gee)[3]; //= malloc(12); // what's this?

	printf("sizeof(gee) is %u, gee is %p, *gee is %p\n", sizeof(gee), gee, *gee);

	printf("sizeof(*gee) is %u\n", sizeof(*gee));

	int cee[0] = {};

	printf("sizeof(cee) is %u, cee is %p\n", sizeof(cee), cee);

	int i;
	for (i = 0; i < 3; i++)
		puts(*(cent+i));

	return 0;
}