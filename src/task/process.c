#include "process.h"
#include "config.h"
#include "status.h"
#include "fs/file.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "string/string.h"
#include "video/video.h"
#include "memory/idt/idt.h"
#include "task.h"
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

void process_save(struct process* process)
{
    // Save the video memory back into our processes video memory, so when we switch back
    // we can carry on where we left off
    video_save(process->video);
}

void process_restore(struct process* process)
{
    if (current_process != 0)
    {
        // Since we are switching process the task associated with the current process
        // must have its video memory mapped back to its internal private video memory
        // so that any prints made by this task do not write to the terminal
        task_map_video_memory(current_process->task);
    }
    current_process = process;
    process_is_running = true;

    // Restore the processes video memory back into the main video memory
    // So what was once shown shows again on the screen
    video_restore(process->video);

    // We now need to unmap the restored processes video memory so that it outputs data directly to the terminal
    // rather than its internal private video memory
    task_unmap_video_memory(process->task);
}


int process_switch(struct process *process)
{
    if (current_process != 0)
    {
        // We must save the old process state
        process_save(current_process);
    }
    

    // Restore the given process
    process_restore(process);
    return 0;
}

int process_load_start(const char *path)
{
    int res = 0;
    struct process *process = 0x00;
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

    process->started = true;

    // As we have started a process we should switch to it
    process_switch(process);
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
    _process->video = video_new();
    
    struct task* task = task_new(_process);
    if (ERROR_I(task) <= 0)
    {
        res = ERROR_I(task);
        goto out;
    }

    _process->task = task;
    // We now need to map the process memory into real memory
    ASSERT(paging_map_to(_process->task->page_directory->directory_entry, (void*) COS32_PROGRAM_VIRTUAL_ADDRESS, _process->ptr, paging_align_address(_process->ptr + _process->size), PAGING_PAGE_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_PAGE_WRITEABLE) == 0);
    ASSERT(paging_map_to(_process->task->page_directory->directory_entry, (void*) COS32_PROGRAM_VIRTUAL_STACK_ADDRESS_END, _process->stack, paging_align_address(_process->stack + COS32_USER_PROGRAM_STACK_SIZE), PAGING_ACCESS_FROM_ALL | PAGING_PAGE_PRESENT | PAGING_PAGE_WRITEABLE) == 0);

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
