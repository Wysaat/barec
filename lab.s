section .text
    global _start

_start:
    push   ebp
    mov    ebp, esp
    sub    esp, 16
    lea    ebx, [ebp-4]
    mov    eax, [ebx]
    lea    ebx, [ebp-8]
    mov    eax, [ebx]
    mov    eax, 3
    push   eax
    mov    eax, 4
    pop    ebx
    xchg   eax, ebx
    add    eax, ebx
    lea    ebx, [ebp-12]
    mov    eax, [ebx]
    lea    ebx, [ebp-12]
    mov    eax, [ebx]
    push   ebx
    mov    eax, 4
    pop    ebx
    mov    [ebx], eax
    lea    ebx, [ebp-4]
    mov    eax, [ebx]
    push   ebx
    mov    eax, 423
    pop    ebx
    mov    [ebx], eax
    lea    ebx, [ebp-8]
    mov    eax, [ebx]
    push   ebx
    lea    ebx, [ebp-12]
    mov    eax, [ebx]
    push   eax
    mov    eax, 2
    pop    ebx
    xchg   eax, ebx
    mul    ebx
    push   eax
    lea    ebx, [ebp-4]
    mov    eax, [ebx]
    pop    ebx
    xchg   eax, ebx
    sub    eax, ebx
    push   eax
    mov    eax, 5
    push   eax
    mov    eax, 4
    pop    ebx
    xchg   eax, ebx
    mul    ebx
    pop    ebx
    xchg   eax, ebx
    add    eax, ebx
    pop    ebx
    mov    [ebx], eax
    hlt
