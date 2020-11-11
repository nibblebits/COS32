#ifndef ISR80H_FONT_H
#define ISR80H_FONT_H

struct interrupt_frame;
void* isr80h_command13_font_get(struct interrupt_frame* frame);
void* isr80h_command14_font_draw(struct interrupt_frame *frame);
void* isr80h_command_15_font_make_empty_string(struct interrupt_frame* frame);
#endif