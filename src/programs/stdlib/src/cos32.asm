; Contains routines for talking directly with major COS32 kernel routines

global print
global cos32_getkey
global kernel_information
global cos32_putchar
global cos32_getkeyblock
global cos32_malloc
global cos32_invoke_command

print:
    push ebp
    mov ebp, esp
    mov eax, 1 ; Command 1 = Print
    mov ebx, [ebp+8] ; String to print
    push dword ebx  ; Push it to the stack
    int 0x80 ; Invoke kernel to print
    add esp, 4 ; Restore stack
    pop ebp
    ret

cos32_getkey:
    push ebp
    mov ebp, esp
    mov eax, 2 ; Command 2 = Get key
    int 0x80 ; Invoke COS32 kernel
    ; EAX contains the result
    pop ebp
    ret

cos32_getkeyblock:
    push ebp
    mov ebp, esp
.try_again:
    mov eax, 2 ; Command 2 = Get key
    int 0x80
    cmp eax, 0x00
    je .try_again
    pop ebp
    ret

kernel_information:
    push ebp
    mov ebp, esp
    mov eax, 3 ; Command 3 = Kernel information
    mov ebx, [ebp+8] ; The "struct kern_info" structure
    push dword ebx
    int 0x80
    add esp, 4
    pop ebp
    ret

cos32_putchar:
    push ebp
    mov ebp, esp
    mov eax, 4 ; Command 4 = putchar write to stdout
    mov ebx, [ebp+8] 
    push dword ebx
    int 0x80
    add esp, 4
    pop ebp
    ret


cos32_malloc:
    push ebp
    mov ebp, esp
    mov eax, 5 ; Command 5 = malloc for entire process
    mov ebx, [ebp+8] ; Size in bytes
    push dword ebx
    int 0x80
    add esp, 4
    pop ebp
    ret

cos32_invoke_command:
    push ebp
    mov ebp, esp
    mov eax, 6 ; Command 6 = Invoke command. SYSTEM command essentially
    mov ebx, [ebp+8] ; The pointer to the "command_argument" structure
    push dword ebx
    int 0x80
    add esp, 4
    pop ebp
    ret
    