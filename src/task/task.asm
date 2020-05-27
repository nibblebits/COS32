global user_mode_enter

user_mode_enter:
    mov ebp, esp
    mov ebx, [ebp+4] ; FUNCTION POINTER FOR USER LAND TO RUN
    mov esp, 0x7e00
    cli
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, esp
    push dword 0x23
    push dword eax
    pushf
    pop eax
    or eax, 0x200
    push eax
    push 0x1b
    push ebx ; FUNCTION TO RUN IN USER LAND
    iret