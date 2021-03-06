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
    void *result = (void *)process_run_for_argument(root_command_argument, 0, 0);
    return result;
}

void *isr80h_command7_sleep(struct interrupt_frame *frame)
{
    uint32_t sleep_seconds = task_current_get_stack_item_uint(0);
    task_usleep(task_current(), sleep_seconds * 1000);
    return (void *)0x00;
}

void* isr80h_command19_process_get_arguments(struct interrupt_frame* frame)
{
    struct process* process = task_current()->process;
    struct process_arguments* arguments = task_virtual_address_to_physical(task_current(), task_current_get_stack_item(0));

    // Load the argc, and argv into user passed structure
    process_get_arguments(process, &arguments->argc, &arguments->argv);
    return 0;
}