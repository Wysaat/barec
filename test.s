section .data
global _start
t0:
t3:
dq 423.423400
t4:
dq 5252.220000
section .bss
t1:
section .text
_start:
push ebp
mov ebp, esp
mov eax, 12
sub esp, eax
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
fld qword [t3]
pop ebx
fst dword [ebx]
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
fld qword [t4]
pop ebx
fst dword [ebx]
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
fld dword [eax]
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
fld dword [eax]
fdiv st0, st1
pop ebx
fst dword [ebx]
mov eax, 0
jmp t2
t2:
leave
ret

