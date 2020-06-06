[BITS 32]
[org 0x400000]

; Hello world is in assembly as I dont have the time to write a C wrapper right now


; First label that should be called for our program, our program is a binary file, so it has no header information
; Putting anything above this will cause corruption know what your doing
_start:
    ; In future we will push to the stack instead
    mov eax, 0  ; Function zero = print
  ;  push _message
    int 0x80 ; Call the kernel

    jmp $
    
_message: db 'Hello world! it works!', 0