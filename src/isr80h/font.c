#include "font.h"
#include "task/task.h"
#include "task/process.h"
#include "video/font/font.h"
#include "kernel.h"

void *isr80h_command13_font_get(struct interrupt_frame *frame)
{

    void *font_name_user_space_addr = task_current_get_stack_item(0);
    char buf[1024];
    ASSERT(copy_string_from_task(task_current(), font_name_user_space_addr, buf, sizeof(buf)) == 0);
    return video_font_get(buf);
}

void* isr80h_command14_font_draw(struct interrupt_frame *frame)
{
    struct video_font* font = task_current_get_stack_item(2);
    void *out = task_current_get_stack_item(1);
    void *text_user_space_addr = task_current_get_stack_item(0);

    char buf[1024];
    copy_string_from_task(task_current(), text_user_space_addr, buf, sizeof(buf));
    video_font_draw(font, out, buf);
    return 0;
}

void* isr80h_command_15_font_make_empty_string(struct interrupt_frame* frame)
{
    struct video_font* font = task_current_get_stack_item(1);
    int len = (int)task_current_get_stack_item_uint(0);
    return video_font_make_empty_string_for_process(task_current()->process, font, len);
}
