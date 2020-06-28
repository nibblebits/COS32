[BITS 32]
global idt_load
global enable_interrupts
global disable_interrupts
global isr1h_wrapper
global isr80h_wrapper
global isr_no_interrupt_wrapper
global isr_invalid_tss_wrapper
global isr_segment_not_present_wrapper
global isr_page_fault_wrapper
global isr0_wrapper
global interrupt_pointer_table
extern interrupt_handler
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

%macro interrupt 1
    global int%1
    int%1:
        cli
        pushad
        push dword %1
        call interrupt_handler
        pop eax
        popad
        sti
        iretd
%endmacro

%assign i 0
%rep 512
    interrupt i
%assign i i+1
%endrep


; We need to make a pointer table for all the interrupt entries, these will point to the associated wrappers
%macro interrupt_array_entry 1
    dd int%1
%endmacro

; Pointer table
interrupt_pointer_table:
%assign i 0
%rep 512
    interrupt_array_entry i
%assign i i+1
%endrep


isr0_wrapper:
    cld
    call isr0_handler
    iret

isr80h_wrapper:
    cli
    mov ebx, esp ; We should pass the stack as a pointer so isr 80h can access it
    push ebx
    ; EAX holds our command lets push it
    push eax
    call isr80h_handler
    pop eax
    pop ebx
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
    call isr_no_interrupt
    sti
    iretd