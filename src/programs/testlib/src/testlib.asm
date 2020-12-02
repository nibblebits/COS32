[BITS 32]

global mytestfunction
global _start
_start:
    ret

mytestfunction:
    push ebp
    mov ebp, esp
    mov eax, 50
    pop ebp
    ret