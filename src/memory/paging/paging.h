#ifndef PAGING_H
#define PAGING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PAGING_TOTAL_PER_TABLE 1024
struct paging_4gb_chunk
{
    uint32_t* directory_entry;
};

struct paging_4gb_chunk* paging_new_4gb();
int paging_map(uint32_t *directory, void *virt, void *phys);
void paging_free_4gb(struct paging_4gb_chunk* chunk);
void paging_switch(uint32_t* directory);

extern void paging_load_directory(uint32_t*);
extern void enable_paging();

#endif