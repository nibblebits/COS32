global user_mode_enter
global user_registers


user_mode_enter:
    mov ebp, esp
    mov ebx, [ebp+4] ; FUNCTION POINTER FOR USER LAND TO RUN
    mov eax, [ebp+8] ; Stack address for this process
    cli
    push eax        ; Save stack address for later
    call user_registers ; Switch to the user registers
    pop eax        ; Restore the stack address, we need to push it to the stack
    
    push dword 0x23
    push dword eax
    pushf
    pop eax
    or eax, 0x200
    push eax
    push 0x1b
    push ebx ; FUNCTION TO RUN IN USER LAND
    iret
    
user_registers:
    cli
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    sti
    ret