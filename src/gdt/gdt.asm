[BITS 32]
global gdt_load
extern print_number

gdt_descriptor:
    dw 0x00 ; Size -1
    dd 0x00  ; GDT start address

gdt_load:
    push ebp
    mov ebp, esp
    mov word ax, [ebp+12]
    dec ax
    mov word [gdt_descriptor+0], ax
    mov dword eax, [ebp+8]
    mov dword [gdt_descriptor+4], eax
    cli
    lgdt[gdt_descriptor]

.continue:
    pop ebp
    ret