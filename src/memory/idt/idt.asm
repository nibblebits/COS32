[BITS 32]
global idt_load
global enable_interrupts
global disable_interrupts

idt_load:
    push ebp
    mov ebp, esp
    mov ebx, [ebp+8]
    lidt [ebx]
    pop ebp
    ret

enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret
