[BITS 32]
global _start
global do_something
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

	call kernel_main

	cli
	hlt
	jmp $


