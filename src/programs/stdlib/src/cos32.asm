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
global cos32_video_rectangle_draw_font_data
global cos32_video_rectangle_publish
global cos32_video_rectangle_get
global cos32_get_arguments
global cos32_flush_video_buffer
global cos32_video_clear_flag

; WARNING AVOID USING ALL GENERAL PURPOSE REGISTERS EXCEPT EAX TO RETURN A RESULT
; I AM NOT SURE WHICH REGISTERS GCC RELIES ON THE VALUE BEING THE SAME
; HOWEVER EBX IS CONFIRMED.
print:
    mov eax, 1 ; Command 1 = Print
    push dword [esp+4]  ; Push it to the stack
    int 0x80 ; Invoke kernel to print
    add esp, 4 ; Restore stack
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
    push dword [ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret

cos32_putchar:
    mov eax, 4 ; Command 4 = putchar write to stdout
    push dword [esp+4]
    int 0x80
    add esp, 4
    ret


cos32_malloc:
    push ebp
    mov ebp, esp
    mov eax, 5 ; Command 5 = malloc for entire process
    push dword [ebp+8] ; Size in bytes
    int 0x80
    add esp, 4
    pop ebp
    ret

cos32_invoke_command:
    push ebp
    mov ebp, esp
    mov eax, 6 ; Command 6 = Invoke command. SYSTEM command essentially
    push dword [ebp+8] ; The pointer to the "command_argument" structure
    int 0x80
    add esp, 4
    pop ebp
    ret

cos32_sleep:
    push ebp
    mov ebp, esp
    mov eax, 7 ; Command 7 = Task sleep. The task is put to sleep
    push dword [ebp+8] ; Total milli seconds to sleep for
    int 0x80
    add esp, 4
    pop ebp
    ret

cos32_video_rectangle_new:
    push ebp
    mov ebp, esp
    mov eax, 8 ; Comamnd 8 = Video rectangle new. Creates a new video rectangle
    push dword [ebp+8] ; X
    push dword [ebp+12] ; Y
    push dword [ebp+16] ; Width
    push dword [ebp+20] ; Height
    int 0x80
    add esp, 16
    pop ebp
    ret

cos32_video_rectangle_set_pixel:
    push ebp
    mov ebp, esp
    mov eax, 9 ; Command 9 = video set pixel. Sets a pixel in this rectangle
    push dword [ebp+8] ; The video rectangle
    push dword [ebp+12] ; X coordinate
    push dword [ebp+16] ; Y Coordinate
    push dword [ebp+20] ; The colour of the pixel
    int 0x80
    add esp, 16
    pop ebp
    ret

cos32_video_rectangle_fill:
    push ebp
    mov ebp, esp
    mov eax, 10 ; Command 10 = video fill. Fills the rectangle with a given colour
    push dword [ebp+8] ; The video rectangle
    push dword [ebp+12] ; The colour to fill the rectangle as
    int 0x80
    add esp, 8
    pop ebp
    ret


cos32_video_rectangle_draw_block:
    push ebp
    mov ebp, esp
    mov eax, 11 ; Command 11 draw rectangle block
    push dword [ebp+8] ; The video rectangle
    push dword [ebp+12] ; The pointer to the data to draw the block for
    push dword [ebp+16] ; The absolute x position to draw on the rectangle
    push dword [ebp+20] ; The absolute y position to draw on the rectangle
    push dword [ebp+24]  ; The total rows to draw on the rectangle
    push dword [ebp+28] ; THe pixels per row to draw
    int 0x80
    add esp, 24
    pop ebp
    ret


cos32_video_rectangle_draw_blocks:
    push ebp
    mov ebp, esp
    mov eax, 12 ; Command 12 draw rectangle blocks 
    push dword [ebp+8] ; The video rectangle
    push dword [ebp+12] ; The pointer to the draw to draw the block for
    push dword [ebp+16] ; The absolute x position to draw on the rectangle
    push dword [ebp+20] ; THe absolute y position to draw on the rectangle
    push dword [ebp+24] ; The total rows to draw on the rectangle
    push dword [ebp+28] ; the pixels per row to draw
    push dword [ebp+32] ; THe total amount of pixel blocks in the provided array
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
    push dword [ebp+8] ; The pointer to the font
    push dword [ebp+12] ; THe pointer to the buffer we wish to put the pixel data into
    push dword [ebp+16] ; The pointer to the character array that we wish to draw as pixels
    int 0x80
    add esp, 12
    pop ebp
    ret

cos32_video_font_make_empty_string:
    push ebp
    mov ebp, esp
    mov eax, 15 ; Command 15 create an empty string for font pixel data
    push dword [ebp+8] ; A pointer to the font
    push dword [ebp+12] ; THe pointer to the "len" integer specifying how many characters we need this buffer to store 
    int 0x80
    add esp, 8
    pop ebp
    ret

cos32_video_rectangle_draw_font_data:
    push ebp
    mov ebp, esp
    mov eax, 16 ; Command 16 draws the given font pixel data into the rectangle
    push dword [ebp+8] ; Rectangle
    push dword [ebp+12] ; FOnt : video_font
    push dword [ebp+16] ; The pixel buffer that contains the pixel blocks
    push dword [ebp+20] ; THe absolute x coordiante to tdraw in this rectangle
    push dword [ebp+24] ; The absolute y coordinate to draw in this rectangle
    push dword [ebp+28] ; The total number of pixel characters stored in the buffer
    int 0x80
    add esp, 24
    pop ebp
    ret


cos32_video_rectangle_publish:
    push ebp
    mov ebp, esp
    mov eax, 17 ; Command 17 publishes the given rectangle
    push dword [ebp+8] ; The rectangle
    push dword [ebp+12] ; The address of the name of the rectangle
    int 0x80
    add esp, 8
    pop ebp
    ret


cos32_video_rectangle_get:
    push ebp
    mov ebp, esp
    mov eax, 18 ; Command 18 get the published rectangle by name
    push dword [ebp+8] ; The rectangle name
    int 0x80
    add esp, 4
    pop ebp
    ret

; int cos32_get_arguments(struct process_arguments* arguments);
cos32_get_arguments:
    push ebp
    mov ebp, esp
    mov eax, 19 ; Command 19 get the process arguments, argc, and argv.
    push dword [ebp+8] ; The process_arguments structure to be populated
    int 0x80    ; Invoke the kernel!
    add esp, 4
    pop ebp
    ret

cos32_flush_video_buffer:
    push ebp
    mov ebp, esp
    mov eax, 20 ; Command 20 flush the video buffer
    int 0x80 ; Invoke the kernel!
    pop ebp
    ret

cos32_video_clear_flag:
    push ebp
    mov ebp, esp
    mov eax, 21 ; Command 21 clear video flag
    push dword [ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret