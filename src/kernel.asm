[BITS 32]
global _start
global int_zero
extern kernel_main

CODE_SEG equ 0x08
DATA_SEG equ 0x10
_start:	
	mov    ax, DATA_SEG    
    mov    ds, ax        
    mov    es, ax
    mov    fs, ax
    mov    gs, ax
    mov    ss, ax
    mov    ebp, 0x00200000
    mov    esp, ebp

    ; Disable PIC we dont care for it
    mov al, 0xff
    out 0xa1, al
    out 0x21, al
	call kernel_main

	cli
	hlt
	jmp $


int_zero:
    int 55
    ret