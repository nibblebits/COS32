[BITS 32]
global _start
extern mytestfunction
extern mysecondtestfunction
_start:
    mov eax, 0xffff
    push eax
    call mytestfunction
    call mysecondtestfunction
    jmp $