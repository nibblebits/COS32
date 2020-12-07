#include "library.h"
#include "string/string.h"
#include "memory/memory.h"
#include "memory/kheap.h"
#include "memory/paging/paging.h"
#include "task/task.h"
#include "status.h"
static struct library *library_head = 0;
static struct library *library_tail = 0;

void library_insert(struct library *library)
{
    if (library_head == 0x00)
    {
        library_head = library;
        library_tail = library;
        return;
    }

    library_tail->next = library;
    library_tail = library;
}

struct library *library_new(const char *name)
{
    if (strlen(name) >= LIBRARY_NAME_MAX)
        return 0;

    struct library *library = kzalloc(sizeof(struct library));
    library->symbols = array_create(sizeof(struct symbol));
    library->sections = array_create(sizeof(struct symbol));
    strncpy(library->name, name, sizeof(library->name));

    library_insert(library);
    return library;
}

struct library *library_get(const char *name)
{
    struct library *current = library_head;
    while (current)
    {
        if (strncmp(current->name, name, sizeof(current->name)) == 0)
        {
            break;
        }
        current = current->next;
    }

    return current;
}

void library_insert_section(struct library *library, struct section *section)
{
    array_insert(library->sections, section);
}

void library_insert_symbol(struct library *library, struct symbol *sym)
{
    array_insert(library->symbols, sym);
}

void library_build_address(void *virt, void *phys, struct addr *addr_out)
{
    addr_out->virt = virt;
    addr_out->phys = phys;
}

int library_new_section(struct library *library, const char *name, struct addr *addr, size_t size)
{
    struct section section;
    section.size = size;
    section.library = library;
    strncpy(section.name, name, sizeof(section.name));
    memcpy(&section.addr, addr, sizeof(struct addr));
    library_insert_section(library, &section);
    return 0;
}

struct section *library_get_section(struct library *library, int index)
{
    if (index >= (int)array_total(library->sections))
        return 0;

    return array_get_index(library->sections, index);
}

int library_sections_count(struct library *library)
{
    return array_total(library->sections);
}

int library_new_symbol(struct library *library, const char *name, struct addr *addr)
{
    if (strlen(name) >= SYMBOL_NAME_MAX)
        return -EINVARG;

    struct symbol sym;
    sym.library = library;
    strncpy(sym.name, name, sizeof(sym.name));
    memcpy(&sym.addr, addr, sizeof(struct addr));
    library_insert_symbol(library, &sym);
    return 0;
}

struct symbol *library_get_symbol_by_name(struct library *library, const char *name)
{
    int total = array_total(library->symbols);
    for (int i = 0; i < total; i++)
    {
        struct symbol *sym = array_get_index(library->symbols, i);
        if (strncmp(sym->name, name, sizeof(sym->name)) == 0)
        {
            return sym;
        }
    }

    return 0;
}

int library_map(struct task* task, struct library* library)
{
    int res = 0;
    int total_sections = library_sections_count(library);
    for (int i = 0; i < total_sections; i++)
    {
        struct section* section = library_get_section(library, i);
        res = paging_map_to(task->page_directory->directory_entry, paging_align_to_lower_page(section->addr.virt), paging_align_to_lower_page(section->addr.phys), paging_align_address(section->addr.phys+section->size), PAGING_PAGE_PRESENT | PAGING_ACCESS_FROM_ALL);
        if (res < 0)
            break;
    }
    return res;
}

int library_map_all(struct task *task)
{
    int res = 0;
    struct library *library = library_head;
    while (library)
    {
        res = library_map(task, library);
        if (res < 0)
            break;

        library = library->next;
    }
    return res;
}