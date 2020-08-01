#include "task.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "string/string.h"
#include "memory/idt/idt.h"
#include "process.h"
#include "status.h"
#include "config.h"
#include "kernel.h"
struct task *current_task = 0;
struct task* task_tail = 0;
struct task* task_head = 0;

void user_registers();
int task_switch(struct task *task)
{
    ASSERT(task->page_directory);
    current_task = task;
    paging_switch(task->page_directory->directory_entry);
    return 0;
}

struct task *task_current()
{
    return current_task;
}


void* task_get_stack_item(struct task* task, int index)
{
    void* result = 0;
     // Assert that we are currently paging the kernel, if someone calls us whilst in a process page we will assume an error as we swap back to kernel when we are done, which may have unpredictable results
    ASSERT(is_kernel_page());

    // We assume the stack grows downwards for this implementation to work.
    uint32_t *sp_ptr = (uint32_t *)task->registers.esp;

    // Let's switch to the process page
    process_page();

    result = (void *)sp_ptr[index];

    // We should switch back to the kernel page
    kernel_page();

    return result;
}


int task_save_state(struct task* task, struct interrupt_frame* frame)
{
    task->registers.ip = frame->ip;
    task->registers.cs = frame->cs;
    task->registers.flags = frame->flags;
    task->registers.esp = frame->esp;
    task->registers.ss = frame->ss;
    task->registers.eax = frame->eax;
    task->registers.ebp = frame->ebp;
    task->registers.ebx = frame->ebx;
    task->registers.ecx = frame->ecx;
    task->registers.edi = frame->edi;
    task->registers.edx = frame->edx;
    task->registers.esi = frame->esi;
    return 0;
}

struct task* task_new()
{
    int res = 0;
    struct task* task = kzalloc(sizeof(struct task));
    res = task_init(task);
    if (res != COS32_ALL_OK)
    {
        goto err;
    }
    if (task_head == 0)
    {
        task_head = task;
        task_tail = task;
        goto out;
    }

    task_tail->next = task;
    task_tail = task;
out:
    return task;

err:
    kfree(task);
    return ERROR(res);

}

int task_init(struct task *task)
{
    memset(task, 0, sizeof(struct task));
    // Maps the entire 4GB address space to its self
    task->page_directory = paging_new_4gb(PAGING_ACCESS_FROM_ALL | PAGING_PAGE_PRESENT);
    if (task->page_directory == 0)
    {
        return -EIO;
    }
    

    // Let's setup some register defaults
    task->registers.ip = COS32_PROGRAM_VIRTUAL_ADDRESS;
    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.cs = USER_CODE_SEGMENT;
    task->registers.esp = COS32_PROGRAM_VIRTUAL_STACK_ADDRESS_START;

    // NOTE THE ENTIRE 4GB ADDRESS SPACE IS MAPPED TO ITS SELF AT THIS POINT, KEEP IN MIND WHEN RUNNING UNPRIVILAGED CODE
    // A MALICIOUS PROGRAM COULD INSPECT MEMORY AND READ WHAT IT SHOULDNT BE READING

    return 0;
}