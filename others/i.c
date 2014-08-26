#include <stdio.h>

static char *type_specifiers[] = {
    "void", "char", "short", "int", "long", "float", "double", "signed", "unsigend", 0
};

int is_specifier(char *token)
{
    char **ptr;
    for (ptr = type_specifiers; *ptr; ptr++)
        if (!strcmp(token, *ptr))
            return 1;
    return 0;
}

// int f() {
// 	a = 3;
// 	int a;
// }

int main()
{
	extern x;
	x = 3;
	int a = sizeof(struct s {int a; int b;});
	printf("a is %d\n", a);
	char *b = "hello", *c = "float";
	printf("is_specifier(b) is %d, is_specifier(c) is %d\n", is_specifier(b), is_specifier(c));
	return 0;
}