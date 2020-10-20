; Contains routines for talking directly with major COS32 kernel routines

global print
global cos32_getkey
global kernel_information
global cos32_putchar
global cos32_getkeyblock
global cos32_malloc
global cos32_invoke_command
global cos32_sleep
global cos32_video_rectangle_new
global cos32_video_rectangle_set_pixel
global cos32_video_rectangle_fill

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

cos32_sleep:
    push ebp
    mov ebp, esp
    mov eax, 7 ; Command 7 = Task sleep. The task is put to sleep
    mov ebx, [ebp+8] ; Total milli seconds to sleep for
    push dword ebx
    int 0x80
    add esp, 4
    pop ebp
    ret

cos32_video_rectangle_new:
    push ebp
    mov ebp, esp
    mov eax, 8 ; Comamnd 8 = Video rectangle new. Creates a new video rectangle
    mov ebx, [ebp+8] ; X
    push dword ebx
    mov ebx, [ebp+12] ; Y
    push dword ebx
    mov ebx, [ebp+16] ; Width
    push dword ebx
    mov ebx, [ebp+20] ; Height
    push dword ebx
    int 0x80
    add esp, 16
    pop ebp
    ret

cos32_video_rectangle_set_pixel:
    push ebp
    mov ebp, esp
    mov eax, 9 ; Command 9 = video set pixel. Sets a pixel in this rectangle
    mov ebx, [ebp+8] ; The video rectangle
    push ebx
    mov ebx, [ebp+12] ; X coordinate
    push ebx
    mov ebx, [ebp+16] ; Y coordinate
    push ebx
    mov ebx, [ebp+20] ; The colour of the pixel
    push ebx
    int 0x80
    add esp, 16
    pop ebp
    ret

cos32_video_rectangle_fill:
    push ebp
    mov ebp, esp
    mov eax, 10 ; COmmand 10 = video fill. Fills the rectangle with a given colour
    mov ebx, [ebp+8] ; The video rectangle
    push ebx
    mov ebx, [ebp+12] ; The colour
    push ebx
    int 0x80
    add esp, 8
    pop ebp
    ret