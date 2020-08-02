#include "paging.h"
#include "config.h"
#include "status.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "task/task.h"
#include "task/process.h"
#include "string/string.h"
#include "kernel.h"


void paging_load_directory(uint32_t* directory);

static uint32_t* current_directory = 0;
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

uint32_t* paging_current_directory()
{
    return current_directory;
}

void paging_unmap_all(struct paging_4gb_chunk *chunk)
{
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

uint32_t paging_round_to_lower_page(void *virt)
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

void *paging_align_address(void *ptr)
{
    if ((uint32_t)ptr % COS32_PAGE_SIZE)
    {
        // Not aligned lets align it
        return (void*) ((uint32_t)ptr + COS32_PAGE_SIZE - ((uint32_t)ptr % COS32_PAGE_SIZE));
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

void paging_free_4gb(__attribute__((unused)) struct paging_4gb_chunk *chunk)
{
}

int copy_string_from_user_process(struct process *process, void *virtual, void *phys, int max)
{

    // Let's assert we are on the kernel page, we can't do anything without this being the case. We will assume a bug if its not
    ASSERT(is_kernel_page());

    // We only support copying of strings that are no larger than a page.
    if (max >= COS32_PAGE_SIZE)
    {
        return -EINVARG;
    }

    int res = 0;
    char *tmp = kzalloc(max);
    if (!tmp)
    {
        res = -ENOMEM;
        goto out;
    }

    uint32_t *process_directory = process->task->page_directory->directory_entry;
    // We must map "tmp" into process memory but first lets remember the old value for later
    uint32_t old_entry = paging_get(process_directory, tmp);
    paging_map(process_directory, tmp, tmp, PAGING_PAGE_WRITEABLE | PAGING_PAGE_PRESENT | PAGING_CACHE_DISABLED | PAGING_ACCESS_FROM_ALL);
    paging_switch(process_directory);
    // Now we have switched to the page of the process we can now access the user process address, lets copy it over to the kernel buffer
    strncpy(tmp, virtual, max);
    kernel_page();

    // Restore the old entry as we possibly remapped one that the process was using
    res = paging_set(process_directory, tmp, old_entry);
    if (res < 0)
    {
        res = -EIO;
        goto out_free;
    }

    // Now that we are back on the kernel page lets copy from that "tmp" pointer we made back into the kernel space "phys" address
    strncpy(phys, tmp, max);
out_free:
    kfree(tmp);
out:
    return 0;
}
