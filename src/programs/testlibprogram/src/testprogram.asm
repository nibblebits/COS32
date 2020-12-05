[BITS 32]
global _start
extern mytestfunction
_start:
    mov eax, 0xffff
    push eax
    call mytestfunction
    jmp $