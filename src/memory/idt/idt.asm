[BITS 32]
global idt_load
global enable_interrupts
global disable_interrupts
global isr0_wrapper
global isr_no_interrupt_wrapper
extern isr0_handler
extern isr_no_interrupt

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

isr0_wrapper:
    cld
    call isr0_handler
    iret


isr_no_interrupt_wrapper:
    cld
    call isr_no_interrupt
    iret