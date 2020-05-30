[BITS 32]
global idt_load
global enable_interrupts
global disable_interrupts
global isr0_wrapper
global isr80h_wrapper
global isr_no_interrupt_wrapper
global isr_invalid_tss_wrapper
global isr_segment_not_present_wrapper
global isr_page_fault_wrapper
extern isr0_handler
extern isr_no_interrupt
extern isr80h_handler
extern isr_invalid_tss_handler
extern isr_segment_not_present_handler
extern isr_page_fault_handler

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

isr80h_wrapper:
    cli
    sti
    iretd
isr_invalid_tss_wrapper:
    cld
    call isr_invalid_tss_handler
    iret

isr_page_fault_wrapper:
    cld
    call isr_page_fault_handler
    iret

isr_segment_not_present_wrapper:
    cld
    call isr_segment_not_present_handler
    iret

isr_no_interrupt_wrapper:
    cli
    sti
    iretd