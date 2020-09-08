
; This file is responsible for calling the C main function of the program whos linking against us
; 23 Aug 2020 23:19

global _start
extern main

test:
    dd 0x00

_start:
    mov eax, 0xfffff
    mov [test], eax
    ; Push argc and argv, currently this thing doesnt even exist so we just push nothing...
    push dword 0
    push dword 0
    call main
    ret
