#include "paging.h"
#include "config.h"
#include "status.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "task/task.h"
#include "task/process.h"
#include "string/string.h"
#include "kernel.h"

void paging_load_directory(uint32_t *directory);

static uint32_t *current_directory = 0;

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
        directory[i] = (uint32_t)entry | flags | PAGING_PAGE_WRITEABLE;
    }

    struct paging_4gb_chunk *chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb->directory_entry = directory;
    return chunk_4gb;
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

int paging_get_indexes(__attribute__((unused)) uint32_t *directory, void *virt, uint32_t *directory_index_out, uint32_t *table_index_out)
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
    int res = paging_get_indexes(directory, virt, &directory_index, &table_index);
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
    int res = paging_get_indexes(directory, virt, &directory_index, &table_index);
    if (res < 0)
    {
        return res;
    }

    uint32_t entry = directory[directory_index];
    uint32_t *table = (uint32_t *)(entry & 0xfffff000);
    table[table_index] = val;
    return 0;
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

void paging_switch(uint32_t *directory)
{
    paging_load_directory(directory);
    current_directory = directory;
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

int paging_map_to(uint32_t *directory, void *virt, void *phys, void *phys_end, int flags)
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
    for (int i = 0; i < 1024; i++)
    {
        uint32_t entry = chunk->directory_entry[i];
        uint32_t *table = (uint32_t *)(entry & 0xfffff000);
        kfree(table);
    }

    kfree(chunk->directory_entry);
}
