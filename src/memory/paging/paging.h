#ifndef PAGING_H
#define PAGING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PAGING_CACHE_DISABLED 0b00010000
#define PAGING_WRITE_THROUGH 0b00001000
#define PAGING_ACCESS_FROM_ALL 0b00000100
#define PAGING_PAGE_WRITEABLE 0b00000010
#define PAGING_PAGE_PRESENT 0b00000001

#define PAGING_TOTAL_PER_TABLE 1024

struct process;
struct task;
struct paging_fault;

typedef void(*PAGING_HANDLER_PAST_FAULT)(struct paging_fault *fault);
typedef bool(*PAGING_HANDLER_LIVE_FAULT)(struct paging_fault *fault);

struct paging_fault_handler
{
    /**
     * Past faults are faults that have happend since the last processing of the paging handlers
     * Use this function to figure out which pages has been accessed since the last tick.
     * Useful to know if you need to copy from buffers
     */
    PAGING_HANDLER_PAST_FAULT past_fault;

  
    /**
     * Invoked when theirs a live fault. Should return true if you handled this fault succesfully.
     * If this fault has nothing to do with you then return false
     */
    PAGING_HANDLER_LIVE_FAULT fault;
};

struct paging_fault
{
    // The page address that faulted
    void *address;

    // The chunk responsible for this fault!
    struct paging_4gb_chunk* chunk;

    struct paging_fault *next;
};

struct paging_4gb_chunk
{
    uint32_t *directory_entry;

    // The paging faults in this 4GB paging chunk since last processing
    struct paging_fault *faults;
    struct paging_fault *last_fault;
};


/**
 * Initialises the paging functionality by loading in the page fault handlers
 * E.g the videofaulthandler.
 */
void paging_init();

/**
 * Processes the given paging chunk
 */
void paging_process(struct paging_4gb_chunk* chunk);

struct paging_4gb_chunk *paging_new_4gb(uint8_t flags);

void paging_register_fault_handler(struct paging_fault_handler *handler);

/**
 * Page fault handler, invoked when we have a page fault
 */
void paging_handle_page_fault();

/**
 * Registers a page fault so its ready for processing later on
 */
struct paging_fault* paging_register_fault(struct paging_4gb_chunk *chunk, void *address);
/**
 * Processes the paging fault queue, calling all paging handlers
 */
void paging_process_fault_queue(struct paging_4gb_chunk *chunk);

/**
 * Clears the fault queue
 */
void paging_fault_queue_clear(struct paging_4gb_chunk *chunk);

int paging_map(uint32_t *directory, void *virt, void *phys, int flags);
int paging_map_range(uint32_t *directory, void *virt, void *phys, int count, int flags);

/**
 * Aligns the provided address to a given page size, rounding up where neccessary.
 * The returned address is a page aligned address greater than or equal to the provided pointer
 * 
 * \param ptr The pointer you wish to page align
 */
void *paging_align_address(void *ptr);

/**
 * Maps pages into memory starting at the physical address until the physical end address is reached.
 * Pages are mapped into the address starting at "virt"
 * 
 * All provided addresses must divide into a page and if they do not the system will panic
 * 
 * /param virt The virtual address to start mapping these pages to
 * /param phys The start physical address so map (must divide into a page)
 * /param phys_end The end physical address to map (must divide into a page)
 */
int paging_map_to(uint32_t *directory, void *virt, void *phys, void *phys_end, int flags);

/**
 * Frees the given 4GB paging chunk
 */
void paging_free_4gb(struct paging_4gb_chunk *chunk);

/**
 * Switches the processor to page with the provided directory
 */
void paging_switch(struct paging_4gb_chunk *chunk);
void paging_unmap_all(struct paging_4gb_chunk *chunk);

/**
 * Returns the current page directory this processor is bound to, this only works properly if you switch pages using the paging_switch function
 */
uint32_t *paging_current_directory();

/**
 * Gets the current 4GB chunk selected
 */
struct paging_4gb_chunk* paging_current_chunk();

int paging_get_indexes(void *virt, uint32_t *directory_index_out, uint32_t *table_index_out);

/**
 * Gets the paging flags for given virtual address
 */
int paging_get_flags(uint32_t *directory, void *virt);

/**
 * 
 * Aligns the given valueto the nearest upper page returning a page aligned address to 4096
 * If address is already aligned we do nothing.
 */

uint32_t paging_align_value_to_lower_page(uint32_t val);

/**
 * Aligns the given value to the nearest lower page returning a page aligned address to 4096
 */
uint32_t paging_align_value_to_upper_page(uint32_t val);

/**
 * Rounds the given virtual address to the nearest lower page returning a page aligned address to 4096
 */
uint32_t paging_align_to_lower_page(void *virt);

/**
 * Sets the virtual address to the given value, the value expected should be the physical address
 * ANDED with the flags, use "paging_map" in most cases
 */
int paging_set(uint32_t *directory, void *virt, uint32_t val);
/**
 * Gets the given physical address along with the flags for the given virtual address. 
 */
uint32_t paging_get(uint32_t *directory, void *virt);

/**
 * Gets only the physical address without the flags for the given virtual address
 */
void* paging_get_physical_address(uint32_t* directory, void* virt);

/**
 * Returns true if the given address is page aligned
 */
bool paging_is_address_aligned(void *ptr);

extern void enable_paging();

#endif