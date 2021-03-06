[BITS 32]

section .asm
global gdt_load
extern print_number

gdt_load:
   MOV   EAX, [esp + 4]
   MOV   [gdt_descriptor + 2], EAX
   MOV   AX, [ESP + 8]
   MOV   [gdt_descriptor], AX
   LGDT  [gdt_descriptor]
   RET

section .data
gdt_descriptor:
    dw 0x00 ; Size -1
    dd 0x00  ; GDT start address
