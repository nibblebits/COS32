#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "io/io.h"
#include "kernel.h"
#include "task/tss.h"
#include "task/process.h"
#include "task/task.h"
#include "string/string.h"
#include "keyboard/keyboard.h"
#include "status.h"
struct idt_desc idt_desc[COS32_MAX_INTERRUPTS];
struct idtr_desc idtr_desc;
extern struct tss tss;
void isr0_wrapper();
void isr1h_wrapper();
void isr80h_wrapper();
void isr_invalid_tss_wrapper();
void isr_no_interrupt_wrapper();
void isr_segment_not_present_wrapper();
void isr_page_fault_wrapper();

extern void *interrupt_pointer_table[COS32_MAX_INTERRUPTS];
static INTERRUPT_CALLBACK_FUNCTION interrupt_callbacks[COS32_MAX_INTERRUPTS];

// Our kernel interrupt and divide by zero exception are reserved and cannot be registered by external resources
static int reserved_interrupts[] = {0x80, 0x00};

struct isr80h_function1_print
{
    const char *message;
};

bool idt_is_reserved(int interrupt)
{
    int size_of_array = sizeof(reserved_interrupts) / sizeof(int);
    for (int i = 0; i < size_of_array; i++)
    {
        if (reserved_interrupts[i] == interrupt)
            return true;
    }

    return false;
}


#warning TIMING BUG, IF WE HOLD FUNCTION KEYS ALL WORKS FINE OTHERWISE THEIRS A PROBLEM
void interrupt_handler(int interrupt, struct interrupt_frame* frame)
{
    if (interrupt_callbacks[interrupt] != 0)
    {    
        kernel_page();
        process_mark_running(false);
        task_current_save_state(frame);
        interrupt_callbacks[interrupt]();
        process_mark_running(true);
        task_page();
    }

out:
     // Acknowledge the interrupt
    outb(PIC1, PIC_EOI);
        
}

int idt_function_is_valid(int interrupt, INTERRUPT_CALLBACK_FUNCTION interrupt_callback)
{
    int res = COS32_ALL_OK;
    if (interrupt >= COS32_MAX_INTERRUPTS || interrupt < 0)
    {
        res = -EINVARG;
        goto out;
    }

    if (idt_is_reserved(interrupt))
    {
        res = -EINVARG;
        goto out;
    }

out:
    return res;
}

int idt_register_interrupt_callback(int interrupt, INTERRUPT_CALLBACK_FUNCTION interrupt_callback)
{
    int res = COS32_ALL_OK;
    res = idt_function_is_valid(interrupt, interrupt_callback);
    if (res < 0)
    {
        goto out;
    }
    interrupt_callbacks[interrupt] = interrupt_callback;

out:
    return res;
}

void idt_general_protection_fault(int interrupt)
{
    panic("General Protection Fault!\n");
}

void *isr80h_command1_print(struct interrupt_frame *frame)
{
    // The message to print is the first element on the user stack
    void *msg_user_space_addr = task_current_get_stack_item(0);
    char buf[1024];
    ASSERT(copy_string_from_task(task_current(), msg_user_space_addr, buf, sizeof(buf)) == 0);
    task_print(buf);
    return 0;
}

void *isr80h_command2_get_key(struct interrupt_frame *frame)
{
    char key = keyboard_pop();
    return (void*)((int)key);
}

void *isr80h_handle_command(int command, struct interrupt_frame *frame)
{
    void *result = 0;
    switch (command)
    {
    case SYSTEM_COMMAND_PRINT:
        result = isr80h_command1_print(frame);
        break;

    case SYSTEM_COMMAND_GET_KEY:
        result = isr80h_command2_get_key(frame);
        break;
    };

    return result;
}

void *isr80h_handler(int command, struct interrupt_frame *frame)
{
    void *res = 0;
    // Our interrupt handler may only be called by programs and not the kernel
    kernel_page();
    task_current_save_state(frame);
    res = isr80h_handle_command(command, frame);
    task_page();
    return res;
}

void isr_no_interrupt(struct interrupt_frame frame)
{
    // Let the PIC know we acknowledge the ISR
    outb(PIC1, PIC_EOI);
}

void isr0_handler(struct interrupt_frame frame)
{
    if (process_running())
    {
        // Replace with program termination
        panic("Divide by zero in process\n");
    }

    panic("Divide by zero in kernel");
}

void isr_page_fault_handler(struct interrupt_frame frame)
{
    panic("Unhandled Page Fault\n");
}

void isr_segment_not_present_handler(struct interrupt_frame frame)
{
    panic("Invalid Segment, Segment not present.\n");
}
void isr_invalid_tss_handler(struct interrupt_frame frame)
{
    panic("Bad TSS\n");
}

void idt_page_fault()
{
    panic("Page Fault: Unable to handle because no interrupt handler for page faults exists\n");
}

void idt_set(int i, void *address)
{
    ASSERT(i < COS32_MAX_INTERRUPTS);
    struct idt_desc *ptr = &idt_desc[i];
    ptr->offset_1 = (uint32_t)address & 0x0000ffff;
    ptr->selector = COS32_CODE_SELECTOR;
    ptr->zero = 0x00;
    ptr->type_attr = 0xEE;
    ptr->offset_2 = (uint32_t)address >> 16;
}

void isr_timer(int interrupt)
{
     // Acknowledge the interrupt
    outb(PIC1, PIC_EOI);
    task_next();    
}

void idt_init()
{
    memset(idt_desc, 0, sizeof(idt_desc));
    idtr_desc.limit = COS32_MAX_INTERRUPTS * sizeof(struct idt_desc);
    idtr_desc.base = (uint32_t)idt_desc;

    // Set all the interrupts to equal the address in the interrupt pointer table, upon calling an interrupt
    // the correct interrupt wrapper will be called, see "idt.asm"
    for (int i = 0; i < COS32_MAX_INTERRUPTS; i++)
    {
        idt_set(i, interrupt_pointer_table[i]);
    }

    // Our custom interrupt is reserved and cannot be taken
    idt_set(0x80, isr80h_wrapper);

    // Setup the general protection fault interrupt
    idt_register_interrupt_callback(0x0D, idt_general_protection_fault);

    // Setup the timer interrupt
    idt_register_interrupt_callback(0x20, isr_timer);
    
    idt_load(&idtr_desc);
}