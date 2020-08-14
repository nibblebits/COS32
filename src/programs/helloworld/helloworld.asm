[BITS 32]

global _start
;[org 0x400000]

; Hello world is in assembly as I dont have the time to write a C wrapper right now


; First label that should be called for our program, our program is a binary file, so it has no header information
; Putting anything above this will cause corruption know what your doing
_start:

start:
    mov ecx, 10000000
continue:
    loop continue
    
    ; In future we will push to the stack instead    
    push dword hello
    mov eax, 1
    int 0x80

    jmp start


read_string:
    push ebp
    mov ebp, esp
    mov edi, [ebp+8]
.loop:
    mov eax, 2
    int 0x80
    cmp eax, 13
    je .done
    cmp eax, 0
    je .loop
    stosb
    jmp .loop
.done:
    pop ebp
    ret

section .data
hello: db 'Hello', 0