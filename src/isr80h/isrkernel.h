#ifndef ISR80H_KERNEL_H
#define ISR80H_KERNEL_H


struct interrupt_frame;
void *isr80h_command3_get_kernel_info(struct interrupt_frame *frame);

#endif