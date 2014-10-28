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

// int f(int x) {
//     return x;
// }

// int main()
// {
//     int x;
//     x = 7 * f(8);
//     return 4 * x;
// }

// int f(int x, int y, int z, int flag) {
//     if (flag > 4)
//         return x * y * z;
//     else
//         return x + y + z;
// }

// int main()
// {
//     int a = 4234;
//     int b = -42;
//     int c = f(a,a,a,b);
//     int d = f(b,b,b,a);
//     return d;
// }

// int fib(int n) {
//     if (n < 2)
//         return n;
//     return fib(n-1) + fib(n-2);
// }

// int main()
// {
//     return fib(10);
// }

// int main()
// {
//     int m[4][4] = {
//         { 1024, 97, 1421, 999 },
//         { 7, 24, 11, 7243 },
//         { 5, 66, 972, 2342 },
//         { 143, 32, 432, 222 },
//     };

//     int i, j, res = 0;
//     for (i = 0; i < 4; i++)
//         for (j = 0; j < 4; j++)
//             res += m[i][j];

//     return res;
// }

// struct matrix {
//     int content[4][4];
// };

// int main()
// {
//     struct matrix m = {
//         {
//             { 42, 97, 1421, 999 },
//             { 7, 24, 11, 7243 },
//             { 5, 66, 972, 2342 },
//             { 143, 32, 432, 222 },
//         },
//     };
//     int i, j, res = 0;
//     for (i = 0; i < 4; i++)
//         for (j = 0; j < 4; j++)
//             res += m.content[i][j];
//     return res;
// }

// float f(int x, float y) {
//     return x * y;
// }

// int main() {
//     int a = 432;
//     float b = 73.22227;
//     float c = f(a, b);
//     double d = f(a, b);
//     return 0;
// }

// struct matrix {
//     int content[4][4];
//     float fcontent[3];
// };

// float matrix_sum(struct matrix matrix) {
//     int i, j;
//     float res;
//     res = matrix.fcontent[0];
//     return res;
// }

// int main()
// {
//     struct matrix m = {
//         {
//             { 42, 97, 1421, 999 },
//             { 7, 24, 11, 7243 },
//             { 5, 66, 972, 2342 },
//             { 143, 32, 432, 222 },
//         },
//         { 424.2423, 77432.2423, -924141.772 },
//     };
//     int x = 42;
//     double g = matrix_sum(m) + x;
//     return g;
// }

// int main()
// {
//     float x = 827.2432;
//     int y = x;
//     float z = y;
//     int w = z;
//     return w;
// }

// float f(int x) {
//     return x;
// }

// int main()
// {
//     int x = 424;
//     float y = f(x);
//     return y;
// }

// struct matrix {
//     int content[4][4];
//     float fcontent[3];
// };

// float matrix_sum(struct matrix matrix) {
//     int i, j;
//     int res;
//     res = matrix.fcontent[0];
//     return res;
// }

// int main()
// {
//     struct matrix m = {
//         {
//             { 42, 97, 1421, 999 },
//             { 7, 24, 11, 7243 },
//             { 5, 66, 972, 2342 },
//             { 143, 32, 432, 222 },
//         },
//         { 424.2423, 77432.2423, -924141.772 },
//     };
//     int x = 42;
//     double g = matrix_sum(m) + x;
//     return g;
// }

// struct matrix {
//     int content[4][4];
//     float fcontent[3];
// };

// float matrix_sum(struct matrix matrix) {
//     int i, j, res = 0;
//     for (i = 0; i < 4; i++)
//         for (j = 0; j < 4; j++)
//             res += matrix.content[i][j];
//     res = matrix.fcontent[0];
//     return res;
// }

// int main()
// {
//     struct matrix m = {
//         {
//             { 42, 97, 1421, 999 },
//             { 7, 24, 11, 7243 },
//             { 5, 66, 972, 2342 },
//             { 143, 32, 432, 222 },
//         },
//         { 424.2423, 77432.2423, -924141.772 },
//     };
//     int x = 42;
//     double g = matrix_sum(m) + x;
//     return g;
// }

// int main()
// {
//     int res = 4242;
//     res *= 424.2423;
//     return res;
// }

// int main()
// {
//     int res = 4242;
//     res *= 424.2423;
//     res /= 77432.2423;
//     res += 924141.772;
//     return res;
// }

// struct matrix {
//     int content[4][4];
//     float fcontent[3];
// };

// int main()
// {
//     struct matrix m = {
//         {
//             { 42, 97, 1421, 999 },
//             { 7, 24, 11, 7243 },
//             { 5, 66, 972, 2342 },
//             { 143, 32, 432, 222 },
//         },
//         { 424.2423, 77432.2423, 924141.772 },
//     };
//     int x = 42;
//     int i, j, res = 0;
//     for (i = 0; i < 4; i++)
//         for (j = 0; j < 4; j++)
//             res += m.content[i][j];
//     res *= 424.2423;
//     res /= 77432.2423;
//     res += 924141.772;
//     double g = (float)res + x;
//     return g;
// }

// int main()
// {
//     int x = 42;
//     int a = 5964040;
//     a /= 77432.2423;
//     double d = (float)a + 42;
//     return d;
// }

// struct matrix {
//     int content[4][4];
//     float fcontent[3];
// };

// float matrix_sum(struct matrix matrix) {
//     int i, j, res = 0;
//     for (i = 0; i < 4; i++)
//         for (j = 0; j < 4; j++)
//             res += matrix.content[i][j];
//     res *= 424.2423;
//     res /= 77432.2423;
//     return res;
// }

// int main()
// {
//     struct matrix m = {
//         {
//             { 42, 97, 1421, 999 },
//             { 7, 24, 11, 7243 },
//             { 5, 66, 972, 2342 },
//             { 143, 32, 432, 222 },
//         },
//         { 424.2423, 77432.2423, 924141.772 },
//     };
//     int x = 42;
//     double g = matrix_sum(m) + 42;
//     return g;
// }

// struct matrix {
//     int content[4][4];
//     float fcontent[3];
// };

// float matrix_sum(struct matrix matrix) {
//     int i, j, res = 0;
//     for (i = 0; i < 4; i++)
//         for (j = 0; j < 4; j++)
//             res += matrix.content[i][j];
//     res *= 424.2423;
//     res /= 77432.2423;
//     res += 924141.772;
//     return res;
// }

// int main()
// {
//     struct matrix m = {
//         {
//             { 42, 97, 1421, 999 },
//             { 7, 24, 11, 7243 },
//             { 5, 66, 972, 2342 },
//             { 143, 32, 432, 222 },
//         },
//         { 424.2423, 77432.2423, 924141.772 },
//     };
//     int x = 42;
//     double g = matrix_sum(m) + x;
//     return g;
// }

struct matrix {
    int content[4][4];
    float fcontent[3];
};

float matrix_sum(struct matrix matrix) {
    int i, j, res = 0;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            res += matrix.content[i][j];
    res *= matrix.fcontent[0];
    res /= matrix.fcontent[1];
    res += matrix.fcontent[2];
    return res;
}

int main()
{
    struct matrix m = {
        {
            { 42, 97, 1421, 999 },
            { 7, 24, 11, 7243 },
            { 5, 66, 972, 2342 },
            { 143, 32, 432, 222 },
        },
        { 424.2423, 77432.2423, -924141.772 },
    };
    int x = 42;
    double g = matrix_sum(m) + x;
    return g;
}