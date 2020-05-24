#ifndef IDT_H
#define IDT_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define IDT_TASK_GATE 0x85
#define IDT_INTERRUPT_GATE 0x8E
#define IDT_TRAP_GATE 0x8F

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

void idt_init();
void idt_load(struct idtr_desc* desc);
void enable_interrupts();
void disable_interrupts();
#endif