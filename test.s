section .data
global _start
t0:
section .bss
t1:
section .text
t2:
push ebp
mov ebp, esp
mov eax, 0
sub esp, eax
lea eax, [ebp+4]
mov eax, [eax]
push eax
mov eax, 4
pop ebx
cmp ebx, eax
setg al
movzx eax, al
jnz t5
lea eax, [ebp+4]
mov eax, [eax]
push eax
mov eax, 4
pop ebx
imul ebx
jmp t3
t5:
lea eax, [ebp+4]
mov eax, [eax]
push eax
mov eax, 4
pop ebx
xchg eax, ebx
cdq
idiv ebx
jmp t3
t3:
leave
ret

_start:
push ebp
mov ebp, esp
mov eax, 16
sub esp, eax
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 4234
pop ebx
mov [ebx], eax
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 42
neg eax
pop ebx
mov [ebx], eax
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
push eax
mov eax, t2
call eax
pop ebx
mov [ebx], eax
mov eax, 16
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
push eax
mov eax, t2
call eax
pop ebx
mov [ebx], eax
mov eax, 16
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
jmp t4
t4:
leave
ret

