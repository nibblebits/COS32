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

struct paging_4gb_chunk
{
    uint32_t *directory_entry;
};

struct paging_4gb_chunk *paging_new_4gb(uint8_t flags);
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

void paging_free_4gb(struct paging_4gb_chunk *chunk);
void paging_switch(uint32_t *directory);
void paging_unmap_all(struct paging_4gb_chunk *chunk);

/**
 * Copies the string located at the virtual address provided for the user process into the physical address provided.
 * If "max" is reached then copying of the string stops.
 * Returns 0 on success, below zero is an error
 */
int copy_string_from_user_process(struct process *process, void *virtual, void *phys, int max);

int paging_get_indexes(uint32_t *directory, void *virt, uint32_t *directory_index_out, uint32_t *table_index_out);


/**
 * Rounds the given virtual address to the nearest lower page returning a page aligned address to 4096
 */
uint32_t paging_round_to_lower_page(void* virt);

/**
 * Sets the virtual address to the given value, the value expected should be the physical address
 * ANDED with the flags, use "paging_map" in most cases
 */
int paging_set(uint32_t *directory, void *virt, uint32_t val);
/**
 * Gets the given physical address along with the flags for the given virtual address. 
 */
uint32_t paging_get(uint32_t *directory, void *virt);

extern void paging_load_directory(uint32_t *);
extern void enable_paging();

#endif