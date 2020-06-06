#include "task.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "status.h"

struct task* current_task = 0;
int task_switch(struct task* task)
{
    current_task = task;
    paging_switch(task->page_directory->directory_entry);
    return 0;
}

struct task* task_current()
{
    return current_task;
}

int task_init(struct task* task)
{
    memset(task, 0, sizeof(struct task));
    // Maps the entire 4GB address space to its self
    task->page_directory = paging_new_4gb(PAGING_ACCESS_FROM_ALL | PAGING_PAGE_PRESENT | PAGING_PAGE_WRITEABLE);
    if (task->page_directory == 0)
    {
        return -EIO;
    }


    #warning We can't unmap all the pages because we still need access to the kernel at this point!!!
    // By default the entire task should have access to no memory at all
   // paging_unmap_all(task->page_directory);
    return 0;
}