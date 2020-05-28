#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "io/io.h"
#include "kernel.h"
#include "task/tss.h"
struct idt_desc idt_desc[COS32_MAX_INTERRUPTS];
struct idtr_desc idtr_desc;
extern struct tss tss;
void isr0_wrapper();
void isr_no_interrupt_wrapper();

struct interrupt_frame
{
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t ss;
};

void isr_no_interrupt(struct interrupt_frame frame)
{
    // Let the bus know we have finished the interrupt
    print(itoa(frame.cs));
        outb(0x20, 0x20);

    print("NO INTERRUPT\n");
    while(1) {}
}

void isr0_handler(struct interrupt_frame frame)
{
    print("INT 0");
    while(1) {}
    outb(0x20, 0x20);
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
    ptr->type_attr = IDT_INTERRUPT_GATE;
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

    idt_load(&idtr_desc);
}