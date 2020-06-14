#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "io/io.h"
#include "kernel.h"
#include "task/tss.h"
#include "task/process.h"
#include "string/string.h"

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

struct interrupt_frame
{
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
};


struct isr80h_function1_print
{
    const char* message
};

void isr1h_handler(struct interrupt_frame frame)
{
    print("testing?\n");
}

void isr80h_handler(struct interrupt_frame* frame)
{
    process_mark_running(false);
    // Inaccessible from the kernel page, must get it here
    struct isr80h_function1_print* function1 = (struct isr80h_function1_print*) frame->sp;
    const char* msg_user_space_addr = function1->message;
    kernel_page();

    char buf[1024];
    ASSERT(copy_string_from_user_process(process_current(), msg_user_space_addr, buf, sizeof(buf)) == 0);
    print(buf);
    print("testing?");
    process_page();
    process_mark_running(true);
}

void isr_no_interrupt(struct interrupt_frame frame)
{
    // Let the bus know we have finished the interrupt

    // outb(0x20, 0x20);
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

void idt_init()
{
    memset(idt_desc, 0, sizeof(idt_desc));
    idtr_desc.limit = COS32_MAX_INTERRUPTS * sizeof(struct idt_desc);
    idtr_desc.base = (uint32_t)idt_desc;

    // Set all the interrupts to a default interrupt that does nothing, these are ones we don't bother handling
    for (int i = 0; i < COS32_MAX_INTERRUPTS; i++)
    {
        idt_set(i, isr_no_interrupt_wrapper);
    }

    idt_set(0, isr0_wrapper);
    idt_set(1, isr1h_wrapper);
    idt_set(0x80, isr80h_wrapper);
    idt_set(0x0A, isr_invalid_tss_wrapper);
    idt_set(0x0B, isr_segment_not_present_wrapper);
    idt_set(0x0E, isr_page_fault_wrapper);
    idt_load(&idtr_desc);
}