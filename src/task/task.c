#include "task.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "status.h"

struct task *current_task = 0;
int task_switch(struct task *task)
{
    current_task = task;
    paging_switch(task->page_directory->directory_entry);
    return 0;
}

struct task *task_current()
{
    return current_task;
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
    

    // NOTE THE ENTIRE 4GB ADDRESS SPACE IS MAPPED TO ITS SELF AT THIS POINT, KEEP IN MIND WHEN RUNNING UNPRIVILAGED CODE
    // A MALICIOUS PROGRAM COULD INSPECT MEMORY AND READ WHAT IT SHOULDNT BE READING

    return 0;
}