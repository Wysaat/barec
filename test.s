section .data
global _start
section .bss
section .text
t0:
push ebp
mov ebp, esp
mov eax, 0
sub esp, eax
mov eax, 3
jmp t1
t1:
leave
ret
_start:
push ebp
mov ebp, esp
mov eax, 460
sub esp, eax
t2:
leave
ret
