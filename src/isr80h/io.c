#include "io.h"
#include "task/task.h"
#include "keyboard/keyboard.h"
#include "idt/idt.h"
#include "kernel.h"

void *isr80h_command1_print(struct interrupt_frame *frame)
{ 
    // The message to print is the first element on the user stack
    void *msg_user_space_addr = task_current_get_stack_item(0);
    char buf[1024];
    ASSERT(copy_string_from_task(task_current(), msg_user_space_addr, buf, sizeof(buf)) == 0);
    task_print(buf);
    return 0;
}

void *isr80h_command2_get_key(struct interrupt_frame *frame)
{
    char key = keyboard_pop();
    return (void *)((int)key);
}


void *isr80h_command4_putchar(struct interrupt_frame *frame)
{
    char c = (char)task_current_get_stack_item_uint(0);
    task_putchar(c);
    return 0;
}
