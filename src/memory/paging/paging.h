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
struct paging_4gb_chunk
{
    uint32_t* directory_entry;
};

struct paging_4gb_chunk* paging_new_4gb(uint8_t flags);
int paging_map(uint32_t *directory, void *virt, void *phys, int flags);
int paging_map_range(uint32_t* directory, void* virt, void* phys, int count, int flags);
void paging_free_4gb(struct paging_4gb_chunk* chunk);
void paging_switch(uint32_t* directory);
void paging_unmap_all(struct paging_4gb_chunk *chunk);

extern void paging_load_directory(uint32_t*);
extern void enable_paging();

#endif