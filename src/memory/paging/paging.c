#include "paging.h"
#include "config.h"
#include "status.h"
#include "memory/kheap.h"
//PAGE_DIRECTORY_ENTRY page_directory[1024] __attribute__((aligned(4096)));
//PAGE_TABLE_ENTRY first_page_table[1024] __attribute__((aligned(4096)));

struct paging_4gb_chunk *paging_new_4gb()
{
    uint32_t *directory = kzalloc(sizeof(uint32_t) * 1024);
    for (int i = 0; i < 1024; i++)
    {
        directory[i] = 0x00000002;
    }

    // Now we need 1024 page tables
    int offset = 0;
    for (int i = 0; i < 1024; i++)
    {
        uint32_t *entry = kzalloc(sizeof(uint32_t) * 1024);
        for (int b = 0; b < 1024; b++)
        {
            entry[b] = (offset + (b * COS32_PAGE_SIZE)) | 3;
        }
        offset += (1024 * COS32_PAGE_SIZE);
        directory[i] = (uint32_t)entry | 3;
    }

    struct paging_4gb_chunk *chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb->directory_entry = directory;
    return chunk_4gb;
}

int paging_map(uint32_t *directory, void *virt, void *phys)
{
    // Virtual address and physical must be page aligned
    if (((unsigned int)virt % COS32_PAGE_SIZE) || ((unsigned int)phys % COS32_PAGE_SIZE))
    {
        return -EINVARG;
    }

    uint32_t directory_index = ((uint32_t)virt / (1024 * COS32_PAGE_SIZE));
    uint32_t table_index = ((uint32_t)virt % (1024 * COS32_PAGE_SIZE) / COS32_PAGE_SIZE);

    uint32_t entry = directory[directory_index];
    uint32_t *table = (uint32_t *)(entry & 0xfffff000);
    table[table_index] = (uint32_t)phys | 3;


    return 0;
}

void paging_switch(uint32_t *directory)
{
    paging_load_directory(directory);
}

void paging_free_4gb(struct paging_4gb_chunk *chunk)
{
}
/*
void paging_init()
{
    for (int i = 0; i < 1024; i++)
    {
        page_directory[i] = 0x00000002;
    }

    // holds the physical address where we want to start mapping these pages to.
    // in this case, we want to map these pages to the very beginning of memory.
    unsigned int i;

    //we will fill all 1024 entries in the table, mapping 4 megabytes
    for (i = 0; i < 1024; i++)
    {
        // As the address is page aligned, it will always leave 12 bits zeroed.
        // Those bits are used by the attributes ;)
        first_page_table[i] = (i * 0x1000) | 3; // attributes: supervisor level, read/write, present.
    }

    // attributes: supervisor level, read/write, present
    page_directory[0] = ((unsigned int)first_page_table) | 3;
    first_page_table[1] = (0x100000) | 1;

    paging_load_directory(page_directory);
}*/