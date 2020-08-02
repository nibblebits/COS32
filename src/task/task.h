#ifndef TASK_H
#define TASK_H
#include "config.h"
#include "memory/paging/paging.h"

struct registers
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
};

struct task
{
    // These are all the heap allocations that this process has, if its not NULL then its allocated
    void *allocations[COS32_MAX_PROGRAM_ALLOCATIONS];

    // The page directory for this task. Upon running this task at any moment this page directory should be enabled for paging
    // When we switch to another task this should not be enabled
    struct paging_4gb_chunk *page_directory;

    // When we switch out of user space the process registers are saved in memory
    struct registers registers;

    // True if this task is currently running, if its false then its in a paused state
    bool awake;

    // The next task in the linked list
    struct task* next;
    
};  


struct interrupt_frame;


/**
 * Returns to the given task based on the registers provided.
 * Note that if the registers are wrong the kernel will fault.
 * They must be initialized correctly!
 */
void task_return(struct registers* regs);
void restore_general_purpose_registers(struct registers* regs);

void *task_get_stack_item(struct task *task, int index);
int task_save_state(struct task *task, struct interrupt_frame *frame);
struct task *task_current();
int task_init(struct task *task);
int task_switch(struct task *task);
struct task* task_new();

/**
 * Should be called for the first task we ever run, i.e no tasks exist before us
 */
void task_run_first_ever_task();

/**
 * Switches to the next task, used for multi-tasking purposes, flips back around when it reaches
 * the end of the task queue. No priority is currently implemented
 */
void task_next();

/**
 * Resumes the given task allow it to run once again
 */
void task_resume(struct task* task);

void user_registers();

#endif