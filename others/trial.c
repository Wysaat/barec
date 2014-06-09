#include <stdio.h>
#include <stdlib.h>

typedef struct list {
    int value;
} list;

typedef struct dummy {
    list *list;
} dummy;

int main()
{
    char a;
    int b = a = 3;
    printf("sizeof(a) is %d, a is %d\n", sizeof(a), a);
    printf("sizeof(b) is %d, b is %d\n", sizeof(b), b);
}