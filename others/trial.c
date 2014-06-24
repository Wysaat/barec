#include <stdio.h>

int main()
{
    int a;
    printf("sizeof(a) is %u\n", sizeof(a));
    (char)a; // an expression with a cast is not an lvalue
    printf("sizeof(a) is %u\n", sizeof(a));

    return 0;
}