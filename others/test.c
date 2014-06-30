#include <stdio.h>

typedef struct list list;

struct tom {
    int a;
    int b;
    int c;
};

struct list {
    struct tom t;
    int m;
};

int main()
{
    int a;
    long long b;
    a = 3;
    b = 432;
    int c = a * b;

    return 0;
}