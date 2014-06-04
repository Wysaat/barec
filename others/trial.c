#include <stdio.h>
#include <stdlib.h>

typedef enum a a;

enum a {
    football,
    basketball,
    tennis,
};

int main()
{
    sizeof(int *);
    sizeof(const int **);
    sizeof(a ***);
    sizeof(volatile a);
    sizeof(int const);
    sizeof(const int);
    return 0;
}