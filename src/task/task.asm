global user_mode_enter

user_mode_enter:
    mov ebp, esp
    mov ebx, [ebp+4] ; FUNCTION POINTER FOR USER LAND TO RUN
    push 0xffffffff
    cli
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, 0x400000
    push dword 0x23
    push dword eax
    pushf
    mov eax, 0x3000
    push 0x1b
    push ebx ; FUNCTION TO RUN IN USER LAND
    iret