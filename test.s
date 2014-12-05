section .data
global _start
t0:
dd t1+0
dd t1+12
dd t0+7
section .bss
t1:
resb 12
resb 12
section .text







