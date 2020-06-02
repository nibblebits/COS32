#include "process.h"
#include "config.h"
#include "status.h"
#include "fs/file.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "string/string.h"
#include "kernel.h"


// The current process that is running
struct process* current_process = 0;

static void process_init(struct process* process)
{
    memset(process, 0, sizeof(struct process));
}


struct process* process_current()
{
    return current_process;
}


int process_switch(struct process* process)
{
    current_process = process;
    task_switch(&process->task);
    return 0;
}

int process_start(struct process* process)
{
    if (process->started)
        return -EIO;

    process_switch(process);
    // In the future we will push argc, argv and other arguments
    user_mode_enter((USER_MODE_FUNCTION)(COS32_PROGRAM_VIRTUAL_ADDRESS));
}

int process_load(const char* filename, struct process** process)
{
    int res = 0;
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
    void* ptr = kzalloc(stat.filesize);
    if (!ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    // Now load it in ;)  (Note we are loading it in as one block here, this may be problematic)
    if(fread(ptr, stat.filesize, 1, fd) != 1)
    {
        res = -EIO;
        goto out;
    }

    
    struct process* _process = kzalloc(sizeof(struct process));
    if (!_process)
    {
        res = -ENOMEM;
        goto out;
    }


    process_init(process);
    strncpy(_process->filename, filename, sizeof(_process->filename));
    _process->ptr = ptr;
    _process->size = stat.filesize;
    res = task_init(&_process->task);
    if (res != COS32_ALL_OK)
    {
        goto out;
    }

    // We now need to map the process memory into real memory
    uint32_t required_pages = (_process->size / 4096) + 1;
    ASSERT(paging_map_range(_process->task.page_directory->directory_entry,COS32_PROGRAM_VIRTUAL_ADDRESS, _process->ptr, required_pages, PAGING_PAGE_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_PAGE_WRITEABLE) == 0);
    // We have the program loaded :o 
    *process = _process;
    
out:
    return res;
}