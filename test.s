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
mov eax, 24
sub esp, eax
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
ffree st0
fld qword [t3]
pop ebx
fst dword [ebx]
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
ffree st0
fld qword [t4]
pop ebx
fst dword [ebx]
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
ffree st0
fld dword [eax]
sub esp, 4
fst dword [esp]
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
ffree st0
fld dword [eax]
fadd dword [esp]
pop eax
pop ebx
fst dword [ebx]
mov eax, 16
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
ffree st0
fld dword [eax]
sub esp, 4
fst dword [esp]
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
ffree st0
fld dword [eax]
fsub dword [esp]
pop eax
pop ebx
fst dword [ebx]
mov eax, 20
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
ffree st0
fld dword [eax]
sub esp, 4
fst dword [esp]
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
ffree st0
fld dword [eax]
fmul dword [esp]
pop eax
pop ebx
fst dword [ebx]
mov eax, 24
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
ffree st0
fld dword [eax]
sub esp, 4
fst dword [esp]
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
ffree st0
fld dword [eax]
fdiv dword [esp]
pop eax
pop ebx
fst dword [ebx]
mov eax, 0
jmp t2
t2:
leave
ret

