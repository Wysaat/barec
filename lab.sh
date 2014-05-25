nasm -f elf lab.s -g
ld -m elf_i386 lab.o -o lab -g