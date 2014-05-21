section .text
    global _start

_start:
	mov ecx, 4
	mov eax, .end
	jmp .cont
	db "Hello, world!"
  .cont:
	mov dword [ebp-0], eax
	mov edx, 1024
	ret
  .end
    db "Hello, world"

    ... * XXX

    mov  eax, ...
    push eax

    mov  eax, XXX

    pop  ebx
    mul  ebx