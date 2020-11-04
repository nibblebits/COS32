#include "process.h"
#include "task/task.h"
#include "task/process.h"
#include "idt/idt.h"

void *isr80h_command5_malloc(struct interrupt_frame *frame)
{
    int size = task_current_get_stack_item_uint(0);
    return task_malloc(task_current(), size);
}

void *isr80h_command6_invoke(struct interrupt_frame *frame)
{
    struct command_argument *root_command_argument = task_current_get_stack_item(0);
    void *result = (void *)process_run_for_argument(root_command_argument, task_current()->process, 0);
    return result;
}

void *isr80h_command7_sleep(struct interrupt_frame *frame)
{
    uint32_t sleep_seconds = task_current_get_stack_item_uint(0);
    task_usleep(task_current(), sleep_seconds * 1000);
    return (void *)0x00;
}