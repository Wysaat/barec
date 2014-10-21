// int main()
// {
//     char a = 42;
//     unsigned short b = 5252;
//     long long c = a + b;
//     unsigned d = a - b;
//     int e = a * b;
//     int f = b / a;
//     return a;
// }

// int main()
// {
//     float a = 423.4234;
//     float b = 5252.22;
//     float c = a + b;
//     float d = a - b;
//     float e = a * b;
//     float f = b / a;
//     a++;
//     b--;
//     return 0;
// }

// int main()
// {
//     char *a = "104342";
//     int b = 0;
//     while (*a)
//         b += *a++ - 48;
//     return b;
// }

struct tom {
    char a;
    int b;
    long c;
    long long d;
};

int main()
{
    struct tom tom = { 1, 2, 3, 400000 };
    int g = tom.a + tom.b + tom.c + tom.d;
    return g;
}