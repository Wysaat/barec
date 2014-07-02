#include <stdio.h>
#include <stddef.h>

int f(int x)
{
    struct what {
        int tee[x];
        char gee[x*8];
        int b;
    };
    return sizeof(struct what);
}

int main()
{
    int a[31][47];
    // a = 43242;
    printf("sizeof(a) is %lu, a is %u, a + 1 is %u\n", sizeof(a), a, a + 1);
    return 0;
}