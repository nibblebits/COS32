[BITS 32]
global _start
global int80h

extern kernel_main
global kernel_registers

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


    call setup_pic
	call kernel_main

    ; We rely just on interrupts from here on.
	jmp $

setup_pic:
    ; Initialize some flags in the PIC's
    mov al, 00010001b ; b4=1: Init; b3=0: Edge; b1=0: Cascade; b0=1: Need 4th init step
    out 0x20, al ; Tell master

    mov al, 0x20 ; Master IRQ0 should be on INT 0x20 (Just after intel exceptions)
    out 0x21, al 
    
    mov al, 00000001b ; b4=0: FNM; b3-2=00: Master/Slave set by hardware; b1=0: Not AEOI; b0=1: x86 mode
    out 0x21, al

    ; We don't bother telling the slave, this may cause us problems later
    ret


; Switches the registers to the kernel registers, must be run while the code segment is in ring 0
kernel_registers:
    ; Kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret


; We must ensure this is 16 byte aligned as its part of the text section where all our
; C code will be apart of, GCC requires all functions to be aligned by 16 bytes by default
; If this assembly causes misalignment our kernel will malfunction
; Let's ensure we are aligned, all other assembly files will use the ".asm" section which will be placed
; at the end of the binary, ensuring alignment issues do not happen
TIMES 512-($-$$) db 0
