[BITS 32]

global _start
;[org 0x400000]

; Hello world is in assembly as I dont have the time to write a C wrapper right now


; First label that should be called for our program, our program is a binary file, so it has no header information
; Putting anything above this will cause corruption know what your doing
_start:

start:
    mov ecx, 1000
continue:
    loop continue
    
    ; In future we will push to the stack instead  
    mov ebx, [name]
    push dword ebx
    mov eax, 1
    int 0x80

    add esp, 4
    
    jmp start

section .data
name: db ' hello world!', 0