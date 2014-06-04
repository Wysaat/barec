; converting binary values to hexadecimal strings

section .bss
    bufflen equ 16
    buff:   resb bufflen

section .data
    hex_str: db " 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", 10
    hexlen   equ $-hex_str

    digits:  db "0123456789abcdef"

section .text

global _start  ; entry point for the linker
_start:
    nop  ; gdb bug?

read:
    mov    eax, 3
    mov    ebx, 0
    mov    ecx, buff
    mov    edx, bufflen
    int    0x80          ; sys_read
    mov    ebp, eax
    cmp    eax, 0
    je     done

    mov    esi, buff
    mov    edi, hex_str
    xor    ecx, ecx

scan:
    xor    eax, eax
    mov    edx, ecx
    shl    edx, 1
    add    edx, ecx

    mov    al, byte [esi+ecx]
    mov    ebx, eax

    and    al, 0fh
    mov    al, byte [digits+eax]
    mov    byte [hex_str+edx+2], al

    lea    eax, [ebp-4]
    lea    eax, [buff]

done:
    mov    eax, 1
    mov    ebx, 0
    int    0x80