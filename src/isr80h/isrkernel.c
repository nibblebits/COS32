#include "isrkernel.h"
#include "idt/idt.h"
#include "kernel.h"
#include "task/task.h"

// These symbols are added during linking process automatically with "ld" command
// Note we get the address of these symbols they are the value its self
// DO not try to access the values directly.
extern void *__BUILD_DATE;
extern void *__BUILD_NUMBER;

void *isr80h_command3_get_kernel_info(struct interrupt_frame *frame)
{
    struct kernel_info *kernel_info_struct_user_space_addr = task_current_get_stack_item(0);
    copy_integer_to_task(task_current(), (void *)&kernel_info_struct_user_space_addr->build_no, (int)&__BUILD_NUMBER);
    copy_integer_to_task(task_current(), (void *)&kernel_info_struct_user_space_addr->date, (int)&__BUILD_DATE);
    return 0;
}
