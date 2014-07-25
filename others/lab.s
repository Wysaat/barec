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

    mov [ebx], eax
    mov [ebx], ax

    jmp $
