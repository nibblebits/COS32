#ifndef ISR80H_VIDEO_H
#define ISR80H_VIDEO_H

struct interrupt_frame;
void *isr80h_command8_video_rectangle_new(struct interrupt_frame *frame);
void *isr80h_command9_video_rectangle_set_pixel(struct interrupt_frame *frame);
void* isr80h_command10_video_rectangle_fill(struct interrupt_frame* frame);
void* isr80h_command11_video_rectangle_draw_block(struct interrupt_frame* frame);
void *isr80h_command12_video_rectangle_draw_blocks(struct interrupt_frame *frame);
void* isr80h_command16_rectangle_draw_font_data(struct interrupt_frame* frame);
void* isr80h_command17_rectangle_publish(struct interrupt_frame* frame);
void* isr80h_command18_rectangle_get(struct interrupt_frame* frame);
void* isr80h_command20_video_buffer_flush(struct interrupt_frame* frame);
void* isr80h_command21_video_clear_flag(struct interrupt_frame* frame);
#endif