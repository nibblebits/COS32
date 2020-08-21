#include "heap.h"
#include "memory.h"
#include "kernel.h"
#include "config.h"
#include "status.h"
#include "paging/paging.h"

static int heap_table_entry_type(HEAP_BLOCK_TABLE_ENTRY entry)
{
    // Lower 4 bits are the entry type
    return entry & 0x0f;
}

static int heap_table_validate(void *ptr, void *end, struct heap_table *table)
{
    int res = 0;
    // Let's ensure that the provided start and end pointers match up with the table provided
    size_t table_size = (size_t)(end - ptr);
    size_t total_pages = table_size / COS32_PAGE_SIZE;
    if (table->total != total_pages)
    {
        res = -EINVARG;
        goto out;
    }

out:
    return res;
}

int heap_create(struct heap *heap, void *ptr, void *end, struct heap_table *table)
{
    int res = 0;

    // Heap must always be aligned to the page size
    if (!paging_is_address_aligned(ptr) || !paging_is_address_aligned(end))
    {
        res = -EINVARG;
        goto out;
    }
    memset(heap, 0, sizeof(struct heap));
    heap->saddr = ptr;
    heap->table = table;

    res = heap_table_validate(ptr, end, table);
    if (ISERR(res))
    {
        goto out;
    }

    // Let's initialize the table entires to free blocks!
    size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;
    memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);

    // We don't care about the "end" address at this point, we don't need it we have the valid table now
    // We just needed it for validation of the table
out:
    return res;
}

int heap_get_start_block(struct heap *heap, int total_blocks)
{
    struct heap_table *table = heap->table;
    int bc = 0;
    int bs = -1;
    for (size_t i = 0; i < table->total; i++)
    {
        if (heap_table_entry_type(table->entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE)
        {
            bc = 0;
            bs = -1;
            continue;
        }
        //  If this is the first block thats free then set the block start
        if (bs == -1)
        {
            bs = i;
        }
        bc++;
        if (bc == total_blocks)
        {
            break;
        }
    }

    if (bs == -1)
    {
        return -ENOMEM;
    }

    return bs;
}

int heap_address_to_block(struct heap* heap, void* address)
{
    return ((int)(address - heap->saddr)) / COS32_PAGE_SIZE;
}

void *heap_block_to_address(struct heap *heap, int block)
{
    return heap->saddr + (block * COS32_PAGE_SIZE);
}

static void heap_ensure_block_bounds(struct heap_table *table, int start_block, int end_block)
{
    ASSERT(start_block >= 0 && start_block < (int)table->total && end_block < (int)table->total && start_block <= end_block);
}

void heap_mark_blocks_taken(struct heap *heap, int start_block, int total_blocks)
{
    int end_block = (start_block + total_blocks) - 1;
    heap_ensure_block_bounds(heap->table, start_block, end_block);

    // The first block for this entry is taken and is the first block
    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;
    if (total_blocks > 1)
    {
        // We have more than one entry in the block so set the has next flag
        entry |= HEAP_BLOCK_HAS_NEXT;
    }

    for (int i = start_block; i <= end_block; i++)
    {
        heap->table->entries[i] = entry;
        entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
        if (i != end_block - 1)
        {
            entry |= HEAP_BLOCK_HAS_NEXT;
        }
    }
}

/**
 * Marks all the blocks that are a part of the starting block as free.
 * Stops when the next block does not have a HEAP_BLOCK_HAS_NEXT flag
 */
void heap_mark_blocks_free(struct heap *heap, int starting_block)
{
    struct heap_table *table = heap->table;
    ASSERT(starting_block < (int)table->total);
    ASSERT(table->entries[starting_block] & HEAP_BLOCK_IS_FIRST);
    for (int i = starting_block; i < (int)table->total; i++)
    {
        HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
        // Mark the block entry as free
        table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
        if (!(entry & HEAP_BLOCK_HAS_NEXT))
        {
            // No more block entries to free for this block? Then we need to break
            break;
        }
    }
}

void *heap_malloc_blocks(struct heap *heap, int total_blocks)
{
    void *address = 0;
    int start_block = heap_get_start_block(heap, total_blocks);
    if (ISERR(start_block))
    {
        goto out;
    }
    //Now that we have our start block lets calculate the address
    address = heap_block_to_address(heap, start_block);
    heap_mark_blocks_taken(heap, start_block, total_blocks);
out:
    return address;
}

void *heap_malloc(struct heap *heap, size_t size)
{
    int total_blocks = paging_align_value_to_upper_page(size) / COS32_PAGE_SIZE;
    return heap_malloc_blocks(heap, total_blocks);
}

void heap_free(struct heap *heap, void *ptr)
{
    // Let's assert no one is passing us garbage...
    ASSERT(ptr >= heap->saddr && paging_is_address_aligned(ptr));
    heap_mark_blocks_free(heap, heap_address_to_block(heap, ptr));
}