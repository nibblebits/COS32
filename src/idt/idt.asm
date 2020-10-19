[BITS 32]

section .asm

global idt_load
global enable_interrupts
global disable_interrupts
global isr1h_wrapper
global isr80h_wrapper

global idt_page_fault
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
extern idt_page_fault_handler


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
        ; INTERRUPT FRAME START
        ; ALREADY PUSHED TO US IS THE USER LAND STACK INFORMATION
        ; uint32_t ip;
        ; uint32_t cs;
        ; uint32_t flags;
        ; uint32_t sp;
        ; uint32_t ss;
        ; Save user land registers
        pushad

        ; INTERRUPT FRAME END 

        ; Push a pointer to the user land registers, and the stack that was passed to us. Essentially push a pointer of the interrupt frame
        push esp
        push dword %1
        call interrupt_handler
        pop eax
        pop ebx
        popad
        iretd
%endmacro

%assign i 0
%rep 512
    interrupt i
%assign i i+1
%endrep



isr0_wrapper:
    cld
    call isr0_handler
    iret

idt_page_fault:
        ; INTERRUPT FRAME START
        ; ALREADY PUSHED TO US IS THE USER LAND STACK INFORMATION
        ; uint32_t ip;
        ; uint32_t cs;
        ; uint32_t flags;
        ; uint32_t sp;
        ; uint32_t ss;
        ; Save user land registers
        pushad

        ; INTERRUPT FRAME END 

        ; Push a pointer to the user land registers, and the stack that was passed to us. Essentially push a pointer of the interrupt frame
        push esp
        call idt_page_fault_handler
        pop eax
        popad

        ; Increment the stack pointer past the error code so operation can continue as normal
        add esp, 4

        iretd

isr80h_wrapper:
cli
    ; INTERRUPT FRAME START
    ; ALREADY PUSHED TO US IS THE USER LAND STACK INFORMATION
    ; uint32_t ip;
    ; uint32_t cs;
    ; uint32_t flags;
    ; uint32_t sp;
    ; uint32_t ss;
    ; Save user land registers
    pushad

    ; INTERRUPT FRAME END 

    ; Push a pointer to the user land registers, and the stack that was passed to us. Essentially push a pointer to the interrupt frame
    push esp

    ; EAX holds our command lets push it
    push eax
    call isr80h_handler
    mov dword[tmp_res], eax
    pop ebx
    pop ebx
    ; Restore user land registers
    popad

    ; Set the EAX register to the return result stored in tmp_res
    mov eax, [tmp_res]
    
    iretd


isr_invalid_tss_wrapper:
    cld
    call isr_invalid_tss_handler
    iret

isr_segment_not_present_wrapper:
    cld
    call isr_segment_not_present_handler
    iret

isr_no_interrupt_wrapper:
    call isr_no_interrupt
    iretd

; We need to make a pointer table for all the interrupt entries, these will point to the associated wrappers
section .data

; Stores the return result of the interrupt operation
tmp_res: dd 0

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