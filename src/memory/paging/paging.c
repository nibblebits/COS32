#include "paging.h"
#include "config.h"
#include "status.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "task/task.h"
#include "task/process.h"
#include "string/string.h"
#include "memory/registers.h"
#include "kernel.h"

void paging_load_directory(uint32_t *directory);

static uint32_t *current_directory = 0;
static struct paging_4gb_chunk *current_chunk = 0;

static struct paging_fault_handler *fault_handlers[COS32_MAX_PAGING_FAULT_HANDLERS];

static bool paging_process_live_fault(struct paging_fault *fault);
static void paging_process_past_fault(struct paging_fault *fault);

void paging_init()
{
}

void paging_process(struct paging_4gb_chunk *chunk)
{
    // Process the fault queue of the given paging chunk
    paging_process_fault_queue(chunk);
}

// FIx the magic numbers.....

struct paging_4gb_chunk *paging_new_4gb(uint8_t flags)
{
    uint32_t *directory = kzalloc(sizeof(uint32_t) * 1024);
    // Now we need 1024 page tables
    int offset = 0;
    for (int i = 0; i < 1024; i++)
    {
        uint32_t *entry = kzalloc(sizeof(uint32_t) * 1024);
        for (int b = 0; b < 1024; b++)
        {
            entry[b] = (offset + (b * COS32_PAGE_SIZE)) | flags;
        }
        offset += (1024 * COS32_PAGE_SIZE);
        directory[i] = (uint32_t)entry | flags | PAGING_PAGE_WRITEABLE | PAGING_ACCESS_FROM_ALL | PAGING_CACHE_DISABLED | PAGING_PAGE_PRESENT;
    }

    struct paging_4gb_chunk *chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb->directory_entry = directory;
    return chunk_4gb;
}

void paging_handle_page_fault()
{
    uint32_t bad_address = registers_cr2();
    struct paging_fault *fault = paging_register_fault(paging_current_chunk(), (void *)bad_address);

    // Let's now let the fault handler know about this live fault
    if (!paging_process_live_fault(fault))
    {
        if (is_kernel_page())
        {
            panic("Unhandled page fault!\n");
        }

        // We have to switch to the kernel page as we need to do kernel stuff
        kernel_page();

        // It was a user process that page faulted?
        // Kill the process
        process_crash(task_current()->process, -1);

        // We have no task to go back too as we just destoryed the process and its tasks
        // so we must call task_next to change the current task
        task_next();
    }
}
static struct paging_fault *paging_new_fault(struct paging_4gb_chunk *chunk)
{
    struct paging_fault *ptr = kzalloc(sizeof(struct paging_fault));
    if (!ptr)
    {
        return 0;
    }

    ptr->chunk = chunk;
    return ptr;
}

static void paging_free_fault(struct paging_fault *fault)
{
    kfree(fault);
}

/**
 * Registers a page fault so its ready for processing later on
 */
struct paging_fault *paging_register_fault(struct paging_4gb_chunk *chunk, void *address)
{
    struct paging_fault *fault = paging_new_fault(chunk);
    if (!fault)
    {
        return 0;
    }

    fault->address = address;
    if (chunk->faults == 0)
    {
        chunk->faults = fault;
        chunk->last_fault = fault;
        return fault;
    }

    chunk->last_fault->next = fault;
    chunk->last_fault = fault;

    return fault;
}

static bool paging_process_live_fault(struct paging_fault *fault)
{
    for (int i = 0; i < COS32_MAX_PAGING_FAULT_HANDLERS; i++)
    {
        if (fault_handlers[i] != 0)
        {
            if (fault_handlers[i]->fault(fault))
            {
                return true;
            }
        }
    }

    return false;
}

static void paging_process_past_fault(struct paging_fault *fault)
{
    for (int i = 0; i < COS32_MAX_PAGING_FAULT_HANDLERS; i++)
    {
        if (fault_handlers[i] != 0)
        {
            fault_handlers[i]->past_fault(fault);
        }
    }
}
/**
 * Processes the paging fault queue, calling all paging handlers
 */
void paging_process_fault_queue(struct paging_4gb_chunk *chunk)
{
    struct paging_fault *fault = chunk->faults;
    while (fault)
    {
        paging_process_past_fault(fault);
        fault = fault->next;
    }
}

/**
 * Clears the fault queue
 */
void paging_fault_queue_clear(struct paging_4gb_chunk *chunk)
{
    struct paging_fault *fault = chunk->faults;
    while (fault)
    {
        paging_free_fault(fault);
        fault = fault->next;
    }
}

struct paging_4gb_chunk *paging_current_chunk()
{
    return current_chunk;
}
uint32_t *paging_current_directory()
{
    return current_directory;
}

void paging_unmap_all(struct paging_4gb_chunk *chunk)
{
    // Get rid of magic numbers, at some point.
    for (int i = 0; i < 1024; i++)
    {
        uint32_t entry = chunk->directory_entry[i];
        uint32_t *table = (uint32_t *)(entry & 0xfffff000);
        for (int b = 0; b < 1024; b++)
        {
            table[b] = 0x00;
        }
    }
}

int paging_map_range(uint32_t *directory, void *virt, void *phys, int count, int flags)
{
    int res = 0;
    for (int i = 0; i < count; i++)
    {
        res = paging_map(directory, virt, phys, flags);
        if (res < 0)
            break;
        virt += COS32_PAGE_SIZE;
        phys += COS32_PAGE_SIZE;
    }

    return res;
}

int paging_get_indexes(void *virt, uint32_t *directory_index_out, uint32_t *table_index_out)
{
    // Addresses must be 4096 aligned
    if ((unsigned int)virt % COS32_PAGE_SIZE)
    {
        return -EINVARG;
    }

    *directory_index_out = ((uint32_t)virt / (COS32_TOTAL_PAGE_ENTRIES_PER_DIRECTORY * COS32_PAGE_SIZE));
    *table_index_out = ((uint32_t)virt % (COS32_TOTAL_PAGE_ENTRIES_PER_DIRECTORY * COS32_PAGE_SIZE) / COS32_PAGE_SIZE);
    return 0;
}

uint32_t paging_round_value_to_lower_page(uint32_t val)
{
    val -= (val % COS32_PAGE_SIZE);
    return val;
}

uint32_t paging_align_value_to_upper_page(uint32_t val)
{
    if ((val % COS32_PAGE_SIZE) == 0)
    {
        return val;
    }

    // First we round it to the lower page, we know its misaligned...
    uint32_t new_val = paging_round_value_to_lower_page(val);
    // Since we are sure  the original value is misaligned we need to add on a new page
    // If we don't add the new page on we actually lose information, we have to add a new page
    // Now we know new_val is aligned to the lower page, lets add a new page, alignment will maintain
    new_val += COS32_PAGE_SIZE;

    return new_val;
}

uint32_t paging_align_to_lower_page(void *virt)
{
    uint32_t addr = (uint32_t)virt;
    addr -= (addr % COS32_PAGE_SIZE);
    return addr;
}

uint32_t paging_get(uint32_t *directory, void *virt)
{
    // Addresses must be 4096 aligned
    ASSERT(((unsigned int)virt % COS32_PAGE_SIZE) == 0);

    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    int res = paging_get_indexes(virt, &directory_index, &table_index);
    ASSERT(res >= 0);

    uint32_t entry = directory[directory_index];
    uint32_t *table = (uint32_t *)(entry & 0xfffff000);
    return table[table_index];
}


int paging_set(uint32_t *directory, void *virt, uint32_t val)
{
    // Addresses must be 4096 aligned
    if ((unsigned int)virt % COS32_PAGE_SIZE)
    {
        return -EINVARG;
    }
    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    int res = paging_get_indexes(virt, &directory_index, &table_index);
    if (res < 0)
    {
        return res;
    }

    uint32_t entry = directory[directory_index];
    uint32_t *table = (uint32_t *)(entry & 0xfffff000);
    table[table_index] = val;
    return 0;
}


void* paging_get_physical_address(uint32_t* directory, void* virt)
{
    // We need to pass only aligned virtual addresses to paging_get
    // Let's align this thing.

    void* virt_addr_new = (void*)paging_align_to_lower_page(virt);
    void* difference = (void*)((uint32_t)virt - (uint32_t)virt_addr_new);
    return (void*) ((paging_get(directory, virt_addr_new) & 0xfffff000) + difference);
}

int paging_map(uint32_t *directory, void *virt, void *phys, int flags)
{
    // Virtual address and physical must be page aligned
    if (((unsigned int)virt % COS32_PAGE_SIZE) || ((unsigned int)phys % COS32_PAGE_SIZE))
    {
        return -EINVARG;
    }

    int res = paging_set(directory, virt, (uint32_t)phys | flags);
    return res;
}

void paging_switch(struct paging_4gb_chunk *chunk)
{
    paging_load_directory(chunk->directory_entry);
    current_directory = chunk->directory_entry;
    current_chunk = chunk;
}

bool paging_is_address_aligned(void *ptr)
{
    return ((uint32_t)ptr % COS32_PAGE_SIZE) == 0;
}

void *paging_align_address(void *ptr)
{
    if ((uint32_t)ptr % COS32_PAGE_SIZE)
    {
        // Not aligned lets align it
        return (void *)((uint32_t)ptr + COS32_PAGE_SIZE - ((uint32_t)ptr % COS32_PAGE_SIZE));
    }

    // It's aligned lets return the same pointer provided
    return ptr;
}

int paging_get_flags(uint32_t *directory, void *virt)
{
    return paging_get(directory, virt) & 0xfff;
}

int paging_map_to(uint32_t *directory, void *virt, void *phys, void *phys_end, int flags)
{
    ASSERT(((uint32_t)virt % COS32_PAGE_SIZE) == 0);
    ASSERT(((uint32_t)phys % COS32_PAGE_SIZE) == 0);
    ASSERT(((uint32_t)phys_end % COS32_PAGE_SIZE) == 0);
    ASSERT((uint32_t)phys_end > (uint32_t)phys);

    uint32_t total_bytes = phys_end - phys;
    int total_pages = total_bytes / COS32_PAGE_SIZE;
    return paging_map_range(directory, virt, phys, total_pages, flags);
}

void paging_free_4gb(struct paging_4gb_chunk *chunk)
{
    for (int i = 0; i < 1024; i++)
    {
        uint32_t entry = chunk->directory_entry[i];
        uint32_t *table = (uint32_t *)(entry & 0xfffff000);
        kfree(table);
    }

    kfree(chunk->directory_entry);
}

int paging_find_free_handler_slot_index()
{
    for (int i = 0; i < COS32_MAX_PAGING_FAULT_HANDLERS; i++)
    {
        if (fault_handlers[i] == 0)
        {
            return i;
        }
    }

    return -EIO;
}

/**
 * Registers the given fault handler into the paging system
 */
void paging_register_fault_handler(struct paging_fault_handler *handler)
{
    int free_index = paging_find_free_handler_slot_index();
    if (free_index < 0)
    {
        panic("No free page handler slots available! Recompile the kernel with a higher limit\n");
    }

    fault_handlers[free_index] = handler;
}