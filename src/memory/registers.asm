section .asm

global registers_cr2

registers_cr2:
    push ebp
    mov ebp, esp
    mov eax, cr2
    pop ebp
    ret