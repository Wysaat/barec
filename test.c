3 << 2 + 4 * 5 / 7;
3 > 2 <= 4

3 > 2

mov    eax, 3
mov    ebx, 2
cmp    eax, ebx
jng    tag1
mov    eax, 1
jmp    tag2
tag1:
mov    eax, 0
tag2:
ret