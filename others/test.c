int func(int a, int b) {
    return a * b;
}

int a = func(3, 4); parse_func_call

int a, b, c;
a = 10;
if (a > 2)
    b = 10;
else
    c = 12;

a > 2;
cmp    eax, 0
jz     tag1   // false
b = 10;
jmp    tag2
tag1:
c = 12;
tag2:

while (a > 2)
    b = b + 1;

tag1:
a > 2;
cmp    eax, 0
jz     tag2   // false
b = b + 1;
jmp    tag1
tag2:

for (a = 0; a < 10; a = a + 1)
    b = b + 1;

a = 0; (optional)
tag1:
a < 10; (optional)
cmp    eax, 0
jz     tag2    // false
b = b + 1;
a = a + 1; (optional)
jmp    tag1
tag2:
