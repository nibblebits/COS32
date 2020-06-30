[BITS 32]
[org 0x400000]

; Hello world is in assembly as I dont have the time to write a C wrapper right now


; First label that should be called for our program, our program is a binary file, so it has no header information
; Putting anything above this will cause corruption know what your doing
_start:
    ; In future we will push to the stack instead
    push _message
    call read_string

    push _message
    mov eax, 1
    int 0x80

    jmp $


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

_message: times 512 db 0