#ifndef ISR80H_PROCESS_H
#define ISR80H_PROCESS_H

struct interrupt_frame;
void *isr80h_command5_malloc(struct interrupt_frame *frame);
void *isr80h_command6_invoke(struct interrupt_frame *frame);
void *isr80h_command7_sleep(struct interrupt_frame *frame);

#endif 