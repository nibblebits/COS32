#ifndef IDT_H
#define IDT_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef void(*INTERRUPT_CALLBACK_FUNCTION)();

// ISR Definitions
#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
#define PIC_EOI		0x20		/* End-of-interrupt command code */


#define IDT_TASK_GATE 0x85
#define IDT_INTERRUPT_GATE 0x8E
#define IDT_TRAP_GATE 0x8F


enum SystemCommands
{
    SYSTEM_COMMAND_EXIT,
    SYSTEM_COMMAND_PRINT,
    SYSTEM_COMMAND_GET_KEY,
    SYSTEM_COMMAND_GET_KERNEL_INFO,
    SYSTEM_COMMAND_PUTCHAR
};

struct idt_desc
{
   uint16_t offset_1; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero;      // unused, set to 0
   uint8_t type_attr; // type and attributes, see below
   uint16_t offset_2; // offset bits 16..31
} __attribute__((packed));

struct idtr_desc
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));



struct interrupt_frame
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t reserved;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed));

void idt_init();
void idt_load(struct idtr_desc* desc);
void enable_interrupts();
void disable_interrupts();

int idt_register_interrupt_callback(int interrupt, INTERRUPT_CALLBACK_FUNCTION interrupt_callback);
#endif