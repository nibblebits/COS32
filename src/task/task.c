#include "task.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "string/string.h"
#include "video/video.h"
#include "timer/pit.h"
#include "idt/idt.h"
#include "process.h"
#include "status.h"
#include "config.h"
#include "kernel.h"
struct task *current_task = 0;
struct task *task_tail = 0;
struct task *task_head = 0;

void user_registers();

void task_current_save_state(struct interrupt_frame *frame)
{
    // Assert that we are currently paging the kernel, if someone calls us whilst in a process page we will assume an error as we swap back to kernel when we are done, which may have unpredictable results
    ASSERT(is_kernel_page());

    // Asserts that we have a process
    ASSERT(task_current());

    // Save the registers
    struct task *task = task_current();
    ASSERT(task_save_state(task, frame) == 0);
}

void *task_malloc(struct task *task, int size)
{
    return process_malloc(task->process, size);
}

void task_usleep(struct task *task, uint32_t millis)
{
    task->awake = false;
    task->awake_at = pit_get_millis() + millis;

    // Let's switch to the next task if the current task is us as we are sleeping now and should not return
    if (task == task_current())
    {
        task_next();
    }
}

void task_wake(struct task *task)
{
    task->awake = true;
    task->awake_at = -1;
}

void task_wake_tasks()
{
    struct task *current = task_head;
    while (current != 0)
    {
        if (current->awake || current->awake_at == -1)
        {
            goto next;
        }

        if (pit_get_millis() > current->awake_at)
        {
            task_wake(current);
        }
    next:
        current = current->next;
    }
}

void task_process()
{
    task_wake_tasks();
}

int task_switch(struct task *task)
{
    if (task->page_directory == 0)
    {
        panic("???");
    }
    ASSERT(task->page_directory);
    current_task = task;

    paging_switch(task->page_directory->directory_entry);
    return 0;
}

struct task *task_current()
{
    return current_task;
}

int task_page()
{
    user_registers();
    task_switch(current_task);
    return 0;
}

int copy_integer_to_task(struct task *task, void *address, int val)
{
    int res = 0;
    // Switch to the process page
    paging_switch(task->page_directory->directory_entry);
    *((int *)address) = val;
    // Switch to the kernel page
    kernel_page();

    return res;
}

int copy_string_to_task(struct task *task, void *virtual_address, const char *val, int max)
{
    int res = 0;

    // We only allow strings that are no larger than a page
    if (max >= COS32_PAGE_SIZE)
    {
        return -EINVARG;
    }

    uint32_t *task_directory = task->page_directory->directory_entry;
    char *tmp = kzalloc(max);
    strncpy(tmp, val, max);

    uint32_t old_paging_entry = paging_get(task_directory, tmp);
    paging_map(task_directory, tmp, tmp, PAGING_PAGE_WRITEABLE | PAGING_PAGE_PRESENT | PAGING_CACHE_DISABLED | PAGING_ACCESS_FROM_ALL);

    // Switch to the process page
    paging_switch(task->page_directory->directory_entry);
    // Now we copy the mapped "tmp" variable into the virtual address provided to us
    strncpy(virtual_address, tmp, max);

    kernel_page();

    // Change the process page directory entry back to what it was
    paging_map(task_directory, tmp, tmp, old_paging_entry);

    return res;
}

int copy_string_from_task(struct task *task, void *virtual, void *phys, int max)
{

    // Let's assert we are on the kernel page, we can't do anything without this being the case. We will assume a bug if its not
    ASSERT(is_kernel_page());

    // We only support copying of strings that are no larger than a page.
    if (max >= COS32_PAGE_SIZE)
    {
        return -EINVARG;
    }

    int res = 0;
    char *tmp = kzalloc(max);
    if (!tmp)
    {
        res = -ENOMEM;
        goto out;
    }

    uint32_t *task_directory = task->page_directory->directory_entry;
    // We must map "tmp" into process memory but first lets remember the old value for later
    uint32_t old_entry = paging_get(task_directory, tmp);
    paging_map(task_directory, tmp, tmp, PAGING_PAGE_WRITEABLE | PAGING_PAGE_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(task_directory);
    // Now we have switched to the page of the process we can now access the user process address, lets copy it over to the kernel buffer
    strncpy(tmp, virtual, max);
    kernel_page();

    // Restore the old entry as we possibly remapped one that the process was using
    res = paging_set(task_directory, tmp, old_entry);
    if (res < 0)
    {
        res = -EIO;
        goto out_free;
    }

    // Now that we are back on the kernel page lets copy from that "tmp" pointer we made back into the kernel space "phys" address
    strncpy(phys, tmp, max);
out_free:
    kfree(tmp);
out:
    return 0;
}

uint32_t task_current_get_stack_item_uint(int index)
{
    return (uint32_t)task_current_get_stack_item(index);
}

void *task_current_get_stack_item(int index)
{
    struct task *task = task_current();

    // Assert that we have a task
    ASSERT(task);

    return task_get_stack_item(task, index);
}

void *task_get_stack_item(struct task *task, int index)
{
    void *result = 0;
    // Assert that we are currently paging the kernel, if someone calls us whilst in a process page we will assume an error as we swap back to kernel when we are done, which may have unpredictable results
    ASSERT(is_kernel_page());

    // We assume the stack grows downwards for this implementation to work.
    uint32_t *sp_ptr = (uint32_t *)task->registers.esp;

    // Let's switch to the process page
    task_page();

    result = (void *)sp_ptr[index];

    // We should switch back to the kernel page
    kernel_page();

    return result;
}

int task_save_state(struct task *task, struct interrupt_frame *frame)
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

void task_run_first_ever_task()
{
    ASSERT(current_task == 0);
    ASSERT(task_head);
    task_switch(task_head);
    task_return(&task_head->registers);
}

static struct task *task_get_next_to(struct task *task)
{
    if (!task->next)
    {
        return task_head;
    }

    return task->next;
}

int task_get_count()
{
    int i = 0;
    struct task *task = task_head;
    while (task != 0)
    {
        i++;
        task = task->next;
    }
    return i;
}

int task_get_awake_count()
{
    int i = 0;
    struct task *task = task_head;
    while (task != 0 && task->awake)
    {
        i++;
        task = task->next;
    }
    return i;
}

static struct task *task_get_next_active_task(struct task *task)
{
    struct task *next_task = task_get_next_to(task);
    if (task_get_awake_count() == 0)
    {
        // At least one task always needs to be awake in our implementation
        // This means we need to forcefully awaken one of the tasks, in the future maybe we can combat this issue
        task_wake(next_task);
    }

    if (!next_task->awake)
    {
        next_task = task_get_next_active_task(next_task);
    }

    return next_task;
}

static struct task *task_get_next()
{
    struct task *task = task_get_next_active_task(current_task);
    return task;
}

/**
 * Switches to the next task, used for multi-tasking purposes, flips back around when it reaches
 * the end of the task queue. No priority is currently implemented
 */
void task_next()
{

    struct task *task = task_get_next();

    // Switch the current task and get straight back into user land
    task_switch(task);
    task_return(&task->registers);
}

static void task_list_remove(struct task *task)
{
    if (task->prev)
    {
        task->prev->next = task->next;
    }

    if (task == task_head)
    {
        task_head = task->next;
    }

    if (task == task_tail)
    {
        task_tail = task->prev;
    }

    if (task == current_task)
    {
        current_task = task_get_next();
    }
}

void task_print(const char *message)
{
    // This only works because the task can see the entire kernel address space, bare in mind problems will happen
    // if I change this in the future
    task_page();
    video_terminal_writestring(&current_task->process->video->properties, message);
    kernel_page();
}

void task_putchar(char c)
{
    // This only works because the task can see the entire kernel address space, bare in mind problems will happen
    // if I change this in the future
    task_page();
    video_terminal_putchar(&current_task->process->video->properties, c);
    kernel_page();
}

struct task *task_new(struct process *process)
{
    int res = 0;
    struct task *task = kzalloc(sizeof(struct task));
    if (!task)
    {
        res = -ENOMEM;
        goto out;
    }

    res = task_init(task, process);
    if (res != COS32_ALL_OK)
    {
        goto out;
    }

    if (task_head == 0)
    {
        task_head = task;
        task_tail = task;
        goto out;
    }

    task_tail->next = task;
    task->prev = task_tail;
    task_tail = task;

out:
    if (ISERR(res))
    {
        task_free(task);
        return ERROR(res);
    }

    return task;
}

int task_map_video_memory(struct task *task)
{
    void *video_memory = task->process->video->ptr;
    return paging_map_to(task->page_directory->directory_entry, (void *)COS32_VIDEO_MEMORY_ADDRESS_START, video_memory, paging_align_address(video_memory + COS32_VIDEO_MEMORY_SIZE),
                         PAGING_PAGE_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_PAGE_WRITEABLE);
}

int task_unmap_video_memory(struct task *task)
{
    return paging_map_to(task->page_directory->directory_entry, (void *)COS32_VIDEO_MEMORY_ADDRESS_START, (void *)COS32_VIDEO_MEMORY_ADDRESS_START, paging_align_address((void *)COS32_VIDEO_MEMORY_ADDRESS_START + COS32_VIDEO_MEMORY_SIZE),
                         PAGING_PAGE_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_PAGE_WRITEABLE);
}

int task_init(struct task *task, struct process *process)
{
    ASSERT(process->video);

    memset(task, 0, sizeof(struct task));
    // Maps the entire 4GB address space to its self
    task->page_directory = paging_new_4gb(PAGING_PAGE_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (task->page_directory == 0)
    {
        return -EIO;
    }

    // Let's setup some register defaults

    task->registers.ip = COS32_PROGRAM_VIRTUAL_ADDRESS;
    if (process->filetype == FILE_TYPE_ELF)
    {
        task->registers.ip = elf_get_entry(&process->elf_file->header);
    }

    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.cs = USER_CODE_SEGMENT;
    task->registers.esp = COS32_PROGRAM_VIRTUAL_STACK_ADDRESS_START;

    // This task should be in an awake state
    task->awake = true;

    task->process = process;

    // We must map the real video memory for this task, to its allocated video memory
    // This video memory won't be outputted to the terminal but it will write to a buffer instead

    // Let's map the video memory to the tasks video memory, unless its process is the one in view
    ASSERT(task_map_video_memory(task) == 0);

    // NOTE THE ENTIRE 4GB ADDRESS SPACE IS MAPPED TO ITS SELF AT THIS POINT, KEEP IN MIND WHEN RUNNING UNPRIVILAGED CODE
    // A MALICIOUS PROGRAM COULD INSPECT MEMORY AND READ WHAT IT SHOULDNT BE READING

    return 0;
}

int task_free(struct task *task)
{
    ASSERT(is_kernel_page());

    // Free the paging directory
    paging_free_4gb(task->page_directory);

    // Remove our task from the list
    task_list_remove(task);

    // Finally delete the task memory
    kfree(task);

    return 0;
}