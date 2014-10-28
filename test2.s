section .data
global _start
t0:
t11:
dq 424.242300
t12:
dq 77432.242300
t13:
dq 424.242300
t14:
dq 77432.242300
t15:
dq 924141.772000
section .bss
t1:
section .text
t2:
push ebp
mov ebp, esp
mov eax, 12
sub esp, eax
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 0
pop ebx
mov [ebx], eax
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 0
pop ebx
mov [ebx], eax
jmp t4
t9:
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 0
pop ebx
mov [ebx], eax
jmp t6
t10:
mov eax, 12
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
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 0
push eax
lea eax, [ebp+8]
pop ebx
add eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
pop ebx
add eax, ebx
pop ebx
mov [ebx], eax
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
push eax
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
inc dword [eax]
pop eax
t6:
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
push eax
mov eax, 4
pop ebx
cmp ebx, eax
setl al
movzx eax, al
push eax
mov eax, 0
pop ebx
cmp ebx, eax
setne al
movzx eax, al
cmp eax, 0
jne t10
t7:
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
push eax
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
inc dword [eax]
pop eax
t4:
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
push eax
mov eax, 4
pop ebx
cmp ebx, eax
setl al
movzx eax, al
push eax
mov eax, 0
pop ebx
cmp ebx, eax
setne al
movzx eax, al
cmp eax, 0
jne t9
t5:
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
cdq
push edx
push eax
fild qword [esp]
pop eax
pop edx
sub esp, 8
fst qword [esp]
ffree st0
fld qword [t11]
fmul qword [esp]
pop eax
pop eax
sub esp, 8
fistp qword [esp]
pop eax
pop edx
pop ebx
mov [ebx], eax
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
ffree st0
fld qword [t12]
sub esp, 8
fst qword [esp]
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
cdq
push edx
push eax
fild qword [esp]
pop eax
pop edx
fdiv qword [esp]
pop eax
pop eax
sub esp, 8
fistp qword [esp]
pop eax
pop edx
pop ebx
mov [ebx], eax
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
cdq
push edx
push eax
fild qword [esp]
pop eax
pop edx
jmp t3
t3:
leave
ret

_start:
push ebp
mov ebp, esp
mov eax, 88
sub esp, eax
mov eax, 0
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 0
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 42
pop ebx
mov [ebx], eax
mov eax, 1
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 0
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 97
pop ebx
mov [ebx], eax
mov eax, 2
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 0
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 1421
pop ebx
mov [ebx], eax
mov eax, 3
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 0
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 999
pop ebx
mov [ebx], eax
mov eax, 0
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 1
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 7
pop ebx
mov [ebx], eax
mov eax, 1
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 1
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 24
pop ebx
mov [ebx], eax
mov eax, 2
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 1
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 11
pop ebx
mov [ebx], eax
mov eax, 3
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 1
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 7243
pop ebx
mov [ebx], eax
mov eax, 0
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 2
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 5
pop ebx
mov [ebx], eax
mov eax, 1
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 2
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 66
pop ebx
mov [ebx], eax
mov eax, 2
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 2
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 972
pop ebx
mov [ebx], eax
mov eax, 3
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 2
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 2342
pop ebx
mov [ebx], eax
mov eax, 0
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 3
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 143
pop ebx
mov [ebx], eax
mov eax, 1
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 3
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 32
pop ebx
mov [ebx], eax
mov eax, 2
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 3
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 432
pop ebx
mov [ebx], eax
mov eax, 3
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 3
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
push eax
mov eax, 222
pop ebx
mov [ebx], eax
mov eax, 0
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
push eax
ffree st0
fld qword [t13]
pop ebx
fst dword [ebx]
mov eax, 1
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
push eax
ffree st0
fld qword [t14]
pop ebx
fst dword [ebx]
mov eax, 2
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
push eax
ffree st0
fld qword [t15]
pop ebx
fst dword [ebx]
mov eax, 80
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 42
pop ebx
mov [ebx], eax
mov eax, 88
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 2
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
ffree st0
fld dword [eax]
sub esp, 4
fst dword [esp]
mov eax, 1
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
ffree st0
fld dword [eax]
sub esp, 4
fst dword [esp]
mov eax, 0
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 12
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
ffree st0
fld dword [eax]
sub esp, 4
fst dword [esp]
mov eax, 3
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 3
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 2
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 3
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 1
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 3
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 0
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 3
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 3
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 2
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 2
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 2
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 1
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 2
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 0
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 2
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 3
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 1
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 2
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 1
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 1
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 1
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 0
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 1
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 3
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 0
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 2
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 0
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 1
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 0
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, 0
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, 0
push eax
mov eax, 16
pop ebx
imul ebx
push eax
mov eax, 76
mov ebx, ebp
sub ebx, eax
mov eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
mov eax, [eax]
push eax
mov eax, t2
call eax
add esp, 76
pop ebx
fst qword [ebx]
mov eax, 88
mov ebx, ebp
sub ebx, eax
mov eax, ebx
ffree st0
fld qword [eax]
sub esp, 8
fistp qword [esp]
pop eax
pop edx
jmp t8
t8:
leave
ret

