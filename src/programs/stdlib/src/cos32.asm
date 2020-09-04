; Contains routines for talking directly with major COS32 kernel routines

global print
global kernel_information

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
