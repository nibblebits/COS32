#include "video.h"
#include "idt/idt.h"
#include "task/task.h"
#include "task/process.h"
#include "video/video.h"
#include "video/rectangle.h"

void *isr80h_command8_video_rectangle_new(struct interrupt_frame *frame)
{
    int abs_x = task_current_get_stack_item_uint(3);
    int abs_y = task_current_get_stack_item_uint(2);
    int width = task_current_get_stack_item_uint(1);
    int height = task_current_get_stack_item_uint(0);
    return video_rectangle_new(process_current()->video, abs_x, abs_y, width, height);
}

void *isr80h_command9_video_rectangle_set_pixel(struct interrupt_frame *frame)
{
    struct video_rectangle *rect = task_current_get_stack_item(3);
    int x = task_current_get_stack_item_uint(2);
    int y = task_current_get_stack_item_uint(1);
    int colour = task_current_get_stack_item_uint(0);
    return (void *)video_rectangle_set_pixel(rect, x, y, colour);
}

void* isr80h_command10_video_rectangle_fill(struct interrupt_frame* frame)
{
    struct video_rectangle* rect = task_current_get_stack_item(1);
    int colour = task_current_get_stack_item_uint(0);
    return (void*) video_rectangle_fill(rect, colour);
}