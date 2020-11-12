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
global cos32_video_rectangle_draw_block
global cos32_video_rectangle_draw_blocks
global cos32_video_font_get
global cos32_video_font_draw
global cos32_video_font_make_empty_string

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


cos32_video_rectangle_draw_block:
    push ebp
    mov ebp, esp
    mov eax, 11 ; Command 11 draw rectangle block
    mov ebx, [ebp+8] ; The video rectangle
    push ebx
    mov ebx, [ebp+12] ; The pointer to the data to draw the block for
    push ebx
    mov ebx, [ebp+16] ; The absolute x position to draw on the rectangle
    push ebx
    mov ebx, [ebp+20] ; The absolute y position to draw on the rectangle
    push ebx
    mov ebx, [ebp+24] ; The total rows to draw on the rectangle
    push ebx
    mov ebx, [ebp+28] ; The pixels per row to draw
    push ebx
    int 0x80
    add esp, 24
    pop ebp
    ret


cos32_video_rectangle_draw_blocks:
    push ebp
    mov ebp, esp
    mov eax, 12 ; Command 12 draw rectangle blocks 
    mov ebx, [ebp+8] ; The video rectangle
    push ebx
    mov ebx, [ebp+12] ; The pointer to the data to draw the block for
    push ebx
    mov ebx, [ebp+16] ; The absolute x position to draw on the rectangle
    push ebx
    mov ebx, [ebp+20] ; The absolute y position to draw on the rectangle
    push ebx
    mov ebx, [ebp+24] ; The total rows to draw on the rectangle
    push ebx
    mov ebx, [ebp+28] ; The pixels per row to draw
    push ebx
    mov ebx, [ebp+32] ; The total amount of pixel blocks in the provided array
    int 0x80
    add esp, 28
    pop ebp
    ret

cos32_video_font_get:
    push ebp
    mov ebp, esp
    mov eax, 13 ; Command 13 get video font
    mov ebx, [ebp+8] ; The font name to load
    push ebx
    int 0x80
    add esp, 4
    pop ebp
    ret

cos32_video_font_draw:
    push ebp
    mov ebp, esp
    mov eax, 14 ; Command 14 Draw character pixel data into buffer
    mov ebx, [ebp+8] ; The pointer to the font
    push ebx
    mov ebx, [ebp+12] ; The pointer to the buffer we wish to put pixel data into
    push ebx
    mov ebx, [ebp+16] ; The pointer to the character array that we wish to draw as pixels
    push ebx
    int 0x80
    add esp, 12
    pop ebp
    ret

cos32_video_font_make_empty_string:
    push ebp
    mov ebp, esp
    mov eax, 15 ; Command 15 create an empty string for font pixel data
    mov ebx, [ebp+8] ; A pointer to the font
    push ebx
    mov ebx, [ebp+12] ; The pointer to the "len" integer specifying how many characters we need this buffer to store
    push ebx
    int 0x80
    add esp, 8
    pop ebp
    ret
    