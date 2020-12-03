[BITS 32]
global _start
extern mytestfunction
_start:
    call mytestfunction
    jmp $