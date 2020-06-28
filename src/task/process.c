#include "process.h"
#include "config.h"
#include "status.h"
#include "fs/file.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "string/string.h"
#include "kernel.h"


// The current process that was just running or is running
struct process* current_process = 0;
// This is true if the process is running right now
bool process_is_running = 0;

struct process* processes[COS32_MAX_PROCESSES] = {};

static void process_init(struct process* process)
{
    memset(process, 0, sizeof(struct process));
}


struct process* process_current()
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

int process_page()
{
    task_switch(&current_process->task);
    return 0;
}

int process_switch(struct process* process)
{
    current_process = process;
    process_is_running = true;
    task_switch(&process->task);
    return 0;
}

int process_start(struct process* process)
{
    if (process->started)
        return -EIO;

    process_switch(process);
    // Now that we have switched to the process you should bare in mind its now dangerous to do anything else other than go to user mode

    // In the future we will push argc, argv and other arguments
    user_mode_enter((USER_MODE_FUNCTION)(COS32_PROGRAM_VIRTUAL_ADDRESS), COS32_PROGRAM_VIRTUAL_STACK_ADDRESS_START);
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

struct process* process_get(int index)
{
    return processes[index];
}

int process_load(const char* filename, struct process** process)
{
    int res = 0;
    int process_slot = process_get_free_slot();
    if (process_slot < 0)
    {
        res = -ENOMEM;
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
    void* program_data_ptr = kzalloc(stat.filesize);
    if (!program_data_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    // Now load it in ;)  (Note we are loading it in as one block here, this may be problematic)
    if(fread(program_data_ptr, stat.filesize, 1, fd) != 1)
    {
        res = -EIO;
        goto out;
    }


    // Let's now create a 16K stack
    void* program_stack_ptr = kzalloc(COS32_USER_PROGRAM_STACK_SIZE);
    if (!program_stack_ptr)
    {
        res = -ENOMEM;
        goto out;
    }
    
    struct process* _process = kzalloc(sizeof(struct process));
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
    ASSERT(paging_map_to(_process->task.page_directory->directory_entry, COS32_PROGRAM_VIRTUAL_ADDRESS, _process->ptr, paging_align_address(_process->ptr+_process->size), PAGING_PAGE_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_PAGE_WRITEABLE) == 0);
    ASSERT(paging_map_to(_process->task.page_directory->directory_entry, COS32_PROGRAM_VIRTUAL_STACK_ADDRESS_END, _process->stack, paging_align_address(_process->stack+COS32_USER_PROGRAM_STACK_SIZE), PAGING_ACCESS_FROM_ALL | PAGING_PAGE_PRESENT | PAGING_PAGE_WRITEABLE) == 0);

    // We have the program loaded :o 
    *process = _process;

    // Add the process to the array
    processes[process_slot] = _process;
    
out:
    return res;
}