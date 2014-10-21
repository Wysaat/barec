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

// struct tom {
//     char a;
//     float b;
//     long c;
//     long long d;
// };

// struct tom tom = { 100, 2000.3, 30000, 400000 };

// int main()
// {
//     float g = tom.a + tom.b + tom.c + tom.d;
//     return 3;
// }

// int main()
// {
//     // int a = 3;
//     // int b = 423;
//     // int c = a > b;
//     // int d = a < b;
//     // int e = a == b;
//     // int a = 423;
//     // int b = 3;
//     // int c = a >> b;
//     // int d = a << b;

//     // int a[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
//     // int i, res = 0;
//     // for (i = 0; i < 10; i++)
//     //     res += a[i];
//     // return res;

//     // static int a[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
//     // int i, res = 0;
//     // for (i = 0; i < 10; i++)
//     //     res += a[i];
//     // return res;
// }

int f(int x) {
    if (x > 4)
        return x * 4;
    else
        return x / 4;
}

int main()
{
    int a = 4234;
    int b = -42;
    int c = f(a);
    int d = f(b);
    return d;
}