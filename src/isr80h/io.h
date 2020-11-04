#ifndef ISR80H_IO_H
#define ISR80H_IO_H

struct interrupt_frame;
void *isr80h_command1_print(struct interrupt_frame *frame);
void *isr80h_command2_get_key(struct interrupt_frame *frame);
void *isr80h_command4_putchar(struct interrupt_frame *frame);

#endif