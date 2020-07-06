#include "process.h"
#include "config.h"
#include "status.h"
#include "fs/file.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "string/string.h"
#include "memory/idt/idt.h"
#include "kernel.h"

// The current process that was just running or is running
struct process *current_process = 0;
// This is true if the process is running right now
bool process_is_running = 0;

struct process *processes[COS32_MAX_PROCESSES] = {};

static void process_init(struct process *process)
{
    memset(process, 0, sizeof(struct process));
}

struct process *process_current()
{
    return current_process;
}

void process_mark_running(bool running)
{
    process_is_running = running;
}

bool process_running()
{
    return process_is_running;
}

void process_save_state(struct interrupt_frame *frame)
{
    // Assert that we are currently paging the kernel, if someone calls us whilst in a process page we will assume an error as we swap back to kernel when we are done, which may have unpredictable results
    ASSERT(is_kernel_page());

    // Asserts that we have a process
    ASSERT(process_current());

    // Save the registers
    struct process *proc = process_current();
    proc->registers.ip = frame->ip;
    proc->registers.cs = frame->cs;
    proc->registers.flags = frame->flags;
    proc->registers.sp = frame->sp;
    proc->registers.ss = frame->ss;

    // We are going to have to switch to the current processes page to access these registers
    process_page();
    uint32_t *general_reg_ptr = (uint32_t *)proc->registers.sp;

    // Let's switch back to the kernel page
    kernel_page();
}

void *process_get_stack_item(int index)
{
    void *result = 0;
    struct process *proc = process_current();

    // Assert that we have a process
    ASSERT(proc);

    // Assert that we are currently paging the kernel, if someone calls us whilst in a process page we will assume an error as we swap back to kernel when we are done, which may have unpredictable results
    ASSERT(is_kernel_page());

    // We assume the stack grows downwards for this implementation to work.
    uint32_t *sp_ptr = (uint32_t *)proc->registers.sp;

    // Let's switch to the process page
    process_page();

    result = (void *)sp_ptr[index];

    // We should switch back to the kernel page
    kernel_page();

    return result;
}

int process_page()
{
    user_registers();
    task_switch(&current_process->task);
    return 0;
}

int process_switch(struct process *process)
{
    current_process = process;
    process_is_running = true;
    task_switch(&process->task);
    return 0;
}

int process_load_start(const char *path)
{
    int res = 0;
    struct process *process;
    res = process_load(path, &process);
    if (res < 0)
    {
        return res;
    }

    // Start the process :)
    res = process_start(process);
    return res;
}

int process_start(struct process *process)
{
    if (process->started)
        return -EIO;

    // Before entering user mode we may need to acknolwedge an interrupt on the ISR because we will never return to the caller of process_start
    outb(PIC1, PIC_EOI);

    process_switch(process);
    // Now that we have switched to the process you should bare in mind its now dangerous to do anything else other than go to user mode

    // In the future we will push argc, argv and other arguments
    user_mode_enter((USER_MODE_FUNCTION)(COS32_PROGRAM_VIRTUAL_ADDRESS), COS32_PROGRAM_VIRTUAL_STACK_ADDRESS_START);

    return 0;
}

int process_get_free_slot()
{
    for (int i = 0; i < COS32_MAX_PROCESSES; i++)
    {
        if (processes[i] == 0)
            return i;
    }

    return -1;
}

struct process *process_get(int index)
{
    // Out of bounds
    if (index < 0 || index >= COS32_MAX_PROCESSES)
    {
        return NULL;
    }
    return processes[index];
}

int process_load_for_slot(const char *filename, struct process **process, int process_slot)
{
    int res = 0;

    // A process with the given id is already taken
    if (process_get(process_slot) != 0)
    {
        res = -EISTKN;
        goto out;
    }

    int fd = fopen(filename, "r");
    if (!fd)
    {
        res = -EIO;
        goto out;
    }

    struct file_stat stat;
    res = fstat(fd, &stat);
    if (res != COS32_ALL_OK)
    {
        goto out;
    }

    // Let's create some memory for this program
    void *program_data_ptr = kzalloc(stat.filesize);
    if (!program_data_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    // Now load it in ;)  (Note we are loading it in as one block here, this may be problematic)
    if (fread(program_data_ptr, stat.filesize, 1, fd) != 1)
    {
        res = -EIO;
        goto out;
    }

    // Let's now create a 16K stack
    void *program_stack_ptr = kzalloc(COS32_USER_PROGRAM_STACK_SIZE);
    if (!program_stack_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    struct process *_process = kzalloc(sizeof(struct process));
    if (!_process)
    {
        res = -ENOMEM;
        goto out;
    }

    process_init(_process);
    strncpy(_process->filename, filename, sizeof(_process->filename));
    _process->ptr = program_data_ptr;
    _process->stack = program_stack_ptr;
    _process->size = stat.filesize;
    res = task_init(&_process->task);
    if (res != COS32_ALL_OK)
    {
        goto out;
    }

    // We now need to map the process memory into real memory
    ASSERT(paging_map_to(_process->task.page_directory->directory_entry, COS32_PROGRAM_VIRTUAL_ADDRESS, _process->ptr, paging_align_address(_process->ptr + _process->size), PAGING_PAGE_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_PAGE_WRITEABLE) == 0);
    ASSERT(paging_map_to(_process->task.page_directory->directory_entry, COS32_PROGRAM_VIRTUAL_STACK_ADDRESS_END, _process->stack, paging_align_address(_process->stack + COS32_USER_PROGRAM_STACK_SIZE), PAGING_ACCESS_FROM_ALL | PAGING_PAGE_PRESENT | PAGING_PAGE_WRITEABLE) == 0);

    // We have the program loaded :o
    *process = _process;

    // Add the process to the array
    processes[process_slot] = _process;

out:
    return res;
}

int process_load(const char *filename, struct process **process)
{
    int res = 0;
    int process_slot = process_get_free_slot();
    if (process_slot < 0)
    {
        res = -ENOMEM;
        goto out;
    }

    res = process_load_for_slot(filename, process, process_slot);
out:
    return res;
}
