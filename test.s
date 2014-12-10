section .data
global _start
t0:
dd 8284
dd 74404
dd 44077
dd 66418
dd 33098
dd 53491
dd 35370
dd 79928
dd 53099
dd 52281
dd 75523
dd 92977
dd 57784
dd 60996
dd 78689
dd 49412
dd 45636
dd 91283
dd 7067
dd 39659
dd t0+0
dd 20
section .bss
t1:
section .text




t2:
push ebp
mov ebp, esp
mov eax, 4
sub esp, eax
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
lea eax, [ebp+8]
mov eax, [eax]
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, t0+80
mov eax, [eax]
pop ebx
add eax, ebx
mov eax, [eax]
pop ebx
mov [ebx], eax
lea eax, [ebp+8]
mov eax, [eax]
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, t0+80
mov eax, [eax]
pop ebx
add eax, ebx
push eax
lea eax, [ebp+12]
mov eax, [eax]
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, t0+80
mov eax, [eax]
pop ebx
add eax, ebx
mov eax, [eax]
pop ebx
mov [ebx], eax
lea eax, [ebp+12]
mov eax, [eax]
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, t0+80
mov eax, [eax]
pop ebx
add eax, ebx
push eax
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
pop ebx
mov [ebx], eax
t3:
leave
ret

t4:
push ebp
mov ebp, esp
mov eax, 0
sub esp, eax
lea eax, [ebp+12]
mov eax, [eax]
mov eax, [eax]
push eax
lea eax, [ebp+8]
mov eax, [eax]
mov eax, [eax]
pop ebx
sub eax, ebx
jmp t5
t5:
leave
ret

t6:
push ebp
mov ebp, esp
mov eax, 8
sub esp, eax
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 1
pop ebx
mov [ebx], eax
jmp t8
t17:
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
pop ebx
mov [ebx], eax
jmp t10
t18:
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
push eax
mov eax, 1
push eax
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
pop ebx
sub eax, ebx
push eax
mov eax, t2
call eax
add esp, 8
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
dec dword [eax]
pop eax
t10:
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
push eax
mov eax, 0
pop ebx
cmp ebx, eax
setg al
movzx eax, al
cmp eax, 0
je t19
mov eax, 1
push eax
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
pop ebx
sub eax, ebx
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, t0+80
mov eax, [eax]
pop ebx
add eax, ebx
mov eax, [eax]
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
mov eax, t0+80
mov eax, [eax]
pop ebx
add eax, ebx
mov eax, [eax]
pop ebx
cmp ebx, eax
setg al
movzx eax, al
cmp eax, 0
je t19
mov eax, 1
jmp t20
t19:
mov eax, 0
t20:
push eax
mov eax, 0
pop ebx
cmp ebx, eax
setne al
movzx eax, al
cmp eax, 0
jne t18
t11:
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
t8:
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
push eax
mov eax, t0+84
mov eax, [eax]
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
jne t17
t9:
t7:
leave
ret

t12:
push ebp
mov ebp, esp
mov eax, 8
sub esp, eax
lea eax, [ebp+8]
mov eax, [eax]
push eax
lea eax, [ebp+12]
mov eax, [eax]
pop ebx
cmp ebx, eax
setge al
movzx eax, al
push eax
mov eax, 0
pop ebx
cmp ebx, eax
setne al
movzx eax, al
cmp eax, 0
je t21
jmp t13
t21:
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
lea eax, [ebp+8]
mov eax, [eax]
pop ebx
mov [ebx], eax
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
push eax
mov eax, 1
push eax
lea eax, [ebp+8]
mov eax, [eax]
pop ebx
add eax, ebx
pop ebx
mov [ebx], eax
jmp t14
t22:
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, t0+80
mov eax, [eax]
pop ebx
add eax, ebx
mov eax, [eax]
push eax
lea eax, [ebp+8]
mov eax, [eax]
push eax
mov eax, 4
pop ebx
imul ebx
push eax
mov eax, t0+80
mov eax, [eax]
pop ebx
add eax, ebx
mov eax, [eax]
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
je t23
mov eax, 4
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
mov eax, [eax]
push eax
mov eax, t2
call eax
add esp, 8
t23:
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
t14:
mov eax, 4
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
push eax
lea eax, [ebp+12]
mov eax, [eax]
pop ebx
cmp ebx, eax
setle al
movzx eax, al
push eax
mov eax, 0
pop ebx
cmp ebx, eax
setne al
movzx eax, al
cmp eax, 0
jne t22
t15:
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
push eax
lea eax, [ebp+8]
mov eax, [eax]
push eax
mov eax, t2
call eax
add esp, 8
mov eax, 1
push eax
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
pop ebx
sub eax, ebx
push eax
lea eax, [ebp+8]
mov eax, [eax]
push eax
mov eax, t12
call eax
add esp, 8
lea eax, [ebp+12]
mov eax, [eax]
push eax
mov eax, 1
push eax
mov eax, 8
mov ebx, ebp
sub ebx, eax
mov eax, ebx
mov eax, [eax]
pop ebx
add eax, ebx
push eax
mov eax, t12
call eax
add esp, 8
t13:
leave
ret

_start:
push ebp
mov ebp, esp
mov eax, 0
sub esp, eax
mov eax, 19
push eax
mov eax, 0
push eax
mov eax, t12
call eax
add esp, 8
mov eax, 0
jmp t16
t16:
jmp $

