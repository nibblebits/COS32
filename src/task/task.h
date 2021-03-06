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

struct process;
struct task
{

    // The page directory for this task. Upon running this task at any moment this page directory should be enabled for paging
    // When we switch to another task this should not be enabled
    struct paging_4gb_chunk *page_directory;

    // When we switch out of user space the process registers are saved in memory
    struct registers registers;

    // True if this task is currently running, if its false then its in a paused state
    bool awake;

    // Timestamp of when this task should awake again. Set to zero and awake to false for a permenant sleep that needs manual interruption.
    long awake_at;

    // The process associated with this task
    struct process* process;

    // The next task in the linked list
    struct task* next;

    // The previous task in the linked list
    struct task* prev;
    
};  


struct interrupt_frame;


/**
 * Called frequently by the PIT timer. Should process anything important relating to the task mechnism
 * such as wakeing up processes for example
 */
void task_process();

/**
 * Wakes the given task allowing it to run once again
 */
void task_wake(struct task* task);

/**
 * Pauses the task until the task is unpaused by calling task_wake.
 */
void task_pause(struct task* task);

/**
 * Returns to the given task based on the registers provided.
 * Note that if the registers are wrong the kernel will fault.
 * They must be initialized correctly!
 */
void task_return(struct registers* regs);
void restore_general_purpose_registers(struct registers* regs);


/**
 * Copies the provided integer to the task's virtual address provided
 */
int copy_integer_to_task(struct task* task, void* address, int val);

/**
 * Copys the string to the given task's virtual address
 */
int copy_string_to_task(struct task* task, void* virtual_address, const char* val, int max);


/**
 * Copies the string located at the virtual address provided for the user process into the physical address provided.
 * If "max" is reached then copying of the string stops.
 * Returns 0 on success, below zero is an error
 */
int copy_string_from_task(struct task *task, void *virtual, void *phys, int max);


/**
 * Sets the given stack item index to the given value provided
 */
int task_set_stack_item(struct task* task, int index, uint32_t val);


/**
 * Pushes the item to the stack
 */
int task_push_stack_item(struct task *task, uint32_t val);


/**
 * Returns the given stack item as an unsigned integer
 */
uint32_t task_current_get_stack_item_uint(int index);

/**
 * Returns the given stack item from the tasks stack for the given index
 */
void *task_get_stack_item(struct task *task, int index);


/**
 * Takes a task virtual address and retreives the physical address that the kernel can use.
 */
void* task_virtual_address_to_physical(struct task* task, void* virtual_address);

/**
 * Saves the state for the current task
 */
void task_current_save_state(struct interrupt_frame *frame);
int task_save_state(struct task *task, struct interrupt_frame *frame);
struct task *task_current();
int task_init(struct task *task, struct process* process);
int task_switch(struct task *task);
struct task* task_new(struct process* process);

/**
 * Gets the stack item from the currently running task
 */
void *task_current_get_stack_item(int index);

/**
 * Switches the processor into the task page and swaps back to user segment registers
 */
int task_page();

int task_page_task(struct task* task);


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

/**
 * Prints to video memory for the current task
 */
void task_print(const char* message);

/**
 * Puts one character to the video output buffer for the given task
 */
void task_putchar(char c);

/**
 * Maps the video memory to the given task, when ever page tables are set to this task
 * any write to video memory will write into the tasks video memory so long as the video memory
 * was mapped with this function
 */
int task_map_video_memory(struct task *task);

/**
 * Unmaps the video memory for the given task, video memory is set back to the real video memory addresses,
 * call this if you want the given task to write directly to real video memory causing pixels to appear on the screen
 */
int task_unmap_video_memory(struct task* task);

void user_registers();


/**
 * Allocates some memory for the given task. Shares the memory with the process and all associated tasks
 */
void* task_malloc(struct task* task, int size);


/**
 * Put's the task to sleep for the provided amount of miliseconds
 */
void task_usleep(struct task* task, uint32_t milis);

/**
 * Frees the task provided
 */
int task_free(struct task* task);

#endif