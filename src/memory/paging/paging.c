#include "paging.h"
#include "config.h"
#include "status.h"
#include "memory/kheap.h"
#include "kernel.h"
//PAGE_DIRECTORY_ENTRY page_directory[1024] __attribute__((aligned(4096)));
//PAGE_TABLE_ENTRY first_page_table[1024] __attribute__((aligned(4096)));

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
        directory[i] = (uint32_t)entry | flags;
    }

    struct paging_4gb_chunk *chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb->directory_entry = directory;
    return chunk_4gb;
}


// void paging_map_page_directory_to_user_space(struct paging_4gb_chunk* in_chunk, uint32_t* out)
// {
//     // First map the input chunk
//     paging_map(out, in_chunk, in_chunk, )
//     for (int i = 0; i < 1024; i++)
//     {
//         for (int b = 0; b < 1024; b++)
//         {
//         }
//     }
// }

void paging_unmap_all(struct paging_4gb_chunk *chunk)
{
    for (int i = 0; i < 1024; i++)
    {
        uint32_t entry = chunk->directory_entry[i];
        uint32_t* table = (uint32_t*)(entry & 0xfffff000);
        for (int b = 0; b < 1024; b++)
        {
            table[b] = 0x00;
        }
    }
}


int paging_map_range(uint32_t* directory, void* virt, void* phys, int count, int flags)
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

int paging_map(uint32_t *directory, void *virt, void *phys, int flags)
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
    table[table_index] = (uint32_t)phys | flags;

    return 0;
}

void paging_switch(uint32_t *directory)
{
    paging_load_directory(directory);
}

void* paging_align_address(void* ptr)
{   
    if ((uint32_t)ptr % COS32_PAGE_SIZE)
    {
        // Not aligned lets align it
        return (uint32_t)ptr + COS32_PAGE_SIZE - ((uint32_t)ptr % COS32_PAGE_SIZE);
    }

    // It's aligned lets return the same pointer provided
    return ptr;
}

int paging_map_to(uint32_t* directory, void* virt, void* phys, void* phys_end, int flags)
{
    ASSERT(((uint32_t)virt % COS32_PAGE_SIZE) == 0);
    ASSERT(((uint32_t)phys % COS32_PAGE_SIZE) == 0);
    ASSERT(((uint32_t)phys_end % COS32_PAGE_SIZE) == 0);
    ASSERT((uint32_t)phys_end > (uint32_t)phys);

    int total_bytes = phys_end - phys;
    int total_pages = total_bytes / COS32_PAGE_SIZE;
    return paging_map_range(directory, virt, phys, total_pages, flags);
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