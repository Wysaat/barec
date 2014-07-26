global _start

section .data
    val: dq 123.45

section .bss
    res: resq 1

section .text
    _start:

    fld qword [val]
    fsqrt
    fst qword [res]

    mov eax, 4
    mov ebp, 0xffffeeee
    lea ebx, [ebp+eax]
    lea ebx, [ebp-4]
    mov ebx, ebp
    sub ebx, eax
    mov eax, ebx
    ; mov ebx, [eax+ebp]

    jmp $
