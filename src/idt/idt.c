#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "io/io.h"
#include "kernel.h"
#include "task/tss.h"
#include "task/process.h"
#include "task/task.h"
#include "string/string.h"
#include "keyboard/keyboard.h"
#include "memory/registers.h"
#include "memory/paging/paging.h"
#include "video/rectangle.h"
#include "video/font/font.h"
#include "video/font/formats/psffont.h"
#include "timer/pit.h"
#include "status.h"

ISR80H_COMMAND* isr80h_commands[COS32_MAX_ISR80H_COMMANDS];

struct idt_desc idt_desc[COS32_MAX_INTERRUPTS];
struct idtr_desc idtr_desc;
extern struct tss tss;
void isr0_wrapper();
void isr1h_wrapper();
void isr80h_wrapper();
void idt_page_fault();
void isr_invalid_tss_wrapper();
void isr_no_interrupt_wrapper();
void isr_segment_not_present_wrapper();
void isr_page_fault_wrapper();


extern void *interrupt_pointer_table[COS32_MAX_INTERRUPTS];
static INTERRUPT_CALLBACK_FUNCTION interrupt_callbacks[COS32_MAX_INTERRUPTS];

// Our kernel interrupt is reserved and cannot be registered by external resources
static int reserved_interrupts[] = {0x80};

struct isr80h_function1_print
{
    const char *message;
};

bool idt_is_reserved(int interrupt)
{
    int size_of_array = sizeof(reserved_interrupts) / sizeof(int);
    for (int i = 0; i < size_of_array; i++)
    {
        if (reserved_interrupts[i] == interrupt)
            return true;
    }

    return false;
}



void interrupt_handler(int interrupt, struct interrupt_frame *frame)
{
    kernel_page();
    if (interrupt_callbacks[interrupt] != 0)
    {
        process_mark_running(false);
        task_current_save_state(frame);
        interrupt_callbacks[interrupt]();
        process_mark_running(true);
    }
    task_page();

    // Acknowledge the interrupt
    outb(PIC1, PIC_EOI);
}

int idt_function_is_valid(int interrupt, INTERRUPT_CALLBACK_FUNCTION interrupt_callback)
{
    int res = COS32_ALL_OK;
    if (interrupt >= COS32_MAX_INTERRUPTS || interrupt < 0)
    {
        res = -EINVARG;
        goto out;
    }

    if (idt_is_reserved(interrupt))
    {
        res = -EINVARG;
        goto out;
    }

out:
    return res;
}

int idt_register_interrupt_callback(int interrupt, INTERRUPT_CALLBACK_FUNCTION interrupt_callback)
{
    int res = COS32_ALL_OK;
    res = idt_function_is_valid(interrupt, interrupt_callback);
    if (res < 0)
    {
        goto out;
    }
    interrupt_callbacks[interrupt] = interrupt_callback;

out:
    return res;
}

void idt_general_protection_fault(int interrupt)
{
    // Free the current process
    int id = process_current()->id;
    process_free(process_current());

    // Load the killed program so the user knows this process was killed
    struct process *new_process = 0;
    int res = process_load_for_slot("0:/killed.e", &new_process, id, 0, 0);
    if (res == 0)
    {
        process_start(new_process);
    }

    task_next();
}

void print_page_fault(uint32_t bad_address)
{
    bool was_kernel_page = is_kernel_page();
    uint32_t *faulting_page_directory = paging_current_directory();
    if (faulting_page_directory)
    {
    }
    // SO we don't write to process memory we must switch to the kernel page

    kernel_page();
    print("\n");
    print("ADDRESS: ");
    print(itoa(bad_address));
    print("\n");

    void *bad_page_address = (void *)paging_align_to_lower_page((void *)bad_address);
    uint32_t page_entry = paging_get(task_current()->page_directory->directory_entry, bad_page_address);
    print("Page entry: ");
    print(itoa(page_entry));
    print("\n");

    if (page_entry & PAGING_ACCESS_FROM_ALL)
    {
        print("Access from user space: allowed\n");
    }
    else
    {
        print("Access from user space disallowed\n");
    }

    if (page_entry & PAGING_CACHE_DISABLED)
    {
        print("Cache disabled\n");
    }
    else
    {
        print("Cache enabled\n");
    }

    if (page_entry & PAGING_PAGE_WRITEABLE)
    {
        print("Page is writeable\n");
    }
    else
    {
        print("Page is not writeable\n");
    }

    if (page_entry & PAGING_PAGE_PRESENT)
    {
        print("Page is present\n");
    }
    else
    {
        print("Page is not present\n");
    }

    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    paging_get_indexes(bad_page_address, &directory_index, &table_index);
    print("Directory index:");
    print(itoa(directory_index));
    print("\n");
    print("Table index: ");
    print(itoa(table_index));
    print("\n");

    if (was_kernel_page)
    {
        print("Fault happend on the kernel page\n");
    }
}
void idt_page_fault_handler(struct interrupt_frame frame)
{
    paging_handle_page_fault();
}

#warning "Abstract these functions out the ISR is getting cluttered..."


void isr80h_register_command(int command_id, ISR80H_COMMAND command)
{
    if (command_id < 0 || command_id >= COS32_MAX_ISR80H_COMMANDS)
    {
        panic("The command ID is out of bounds!\n");
    }

    if (isr80h_commands[command_id] != 0)
    {
        panic("Your attempting to register a comamnd thats been taken!\n");
    }
    
    isr80h_commands[command_id] = command;
}
void *isr80h_handle_command(int command, struct interrupt_frame *frame)
{
    void *result = 0;

    if (command <= 0 || command >= COS32_MAX_ISR80H_COMMANDS)
    {
        // Invalid command!
        return 0;
    }

    ISR80H_COMMAND* command_func = isr80h_commands[command];
    if (!command_func)
    {
        // The function does not exist so theirs no command to handle this
        // lets return 
        return 0;
    }

    result = command_func(frame);
    return result;
}

void *isr80h_handler(int command, struct interrupt_frame *frame)
{
    void *res = 0;
    // Our interrupt handler may only be called by programs and not the kernel
    kernel_page();
    task_current_save_state(frame);
    res = isr80h_handle_command(command, frame);

    // If the task is now in a paused state we should switch to the next task
    if (!task_current()->awake)
    {
        task_next();
        // We can never execute past this line, execution is chagned with task_next
    }

    task_page();
    return res;
}

void isr_no_interrupt(struct interrupt_frame frame)
{
    // Let the PIC know we acknowledge the ISR
    outb(PIC1, PIC_EOI);
}

void isr0_handler(struct interrupt_frame frame)
{
    if (process_running())
    {
        // Replace with program termination
        panic("Divide by zero in process\n");
    }

    panic("Divide by zero in kernel");
}

void isr_segment_not_present_handler(struct interrupt_frame frame)
{
    panic("Invalid Segment, Segment not present.\n");
}
void isr_invalid_tss_handler(struct interrupt_frame frame)
{
    panic("Bad TSS\n");
}

void idt_set(int i, void *address)
{
    ASSERT(i < COS32_MAX_INTERRUPTS);
    struct idt_desc *ptr = &idt_desc[i];
    ptr->offset_1 = (uint32_t)address & 0x0000ffff;
    ptr->selector = COS32_CODE_SELECTOR;
    ptr->zero = 0x00;
    ptr->type_attr = 0xEE;
    ptr->offset_2 = (uint32_t)address >> 16;
}

void idt_init()
{
    memset(idt_desc, 0, sizeof(idt_desc));
    idtr_desc.limit = COS32_MAX_INTERRUPTS * sizeof(struct idt_desc);
    idtr_desc.base = (uint32_t)idt_desc;

    // Set all the interrupts to equal the address in the interrupt pointer table, upon calling an interrupt
    // the correct interrupt wrapper will be called, see "idt.asm"
    for (int i = 0; i < COS32_MAX_INTERRUPTS; i++)
    {
        idt_set(i, interrupt_pointer_table[i]);
    }

    // Our custom interrupt is reserved and cannot be taken
    idt_set(0x80, isr80h_wrapper);

    // Setup the general protection fault interrupt
    for (int i = 0; i < 0x20; i++)
    {
        idt_register_interrupt_callback(i, idt_general_protection_fault);
    }

    // Setup page fault handler
    idt_set(0x0e, idt_page_fault);

    // Initialize the PIT timer
    pit_init();
}

void idt_load_now()
{
    idt_load(&idtr_desc);
}