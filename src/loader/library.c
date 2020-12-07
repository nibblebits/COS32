#include "library.h"
#include "string/string.h"
#include "memory/memory.h"
#include "memory/kheap.h"
#include "status.h"
static struct library* library_head = 0;
static struct library* library_tail = 0;

void library_insert(struct library* library)
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

struct library* library_new(const char* name)
{
    if (strlen(name) >= LIBRARY_NAME_MAX)
        return 0;
    

    struct library* library = kzalloc(sizeof(struct library));
    library->symbols = array_create(sizeof(struct symbol));
    library->sections = array_create(sizeof(struct symbol));
    strncpy(library->name, name, sizeof(library->name));
    
    library_insert(library);
    return library;
}

void library_insert_symbol(struct library* library, struct symbol* sym)
{
    array_insert(library->symbols, sym);
}

void library_insert_section(struct library* library, struct section* section)
{
    array_insert(library->sections, section);
}

int library_new_symbol(struct library* library, const char* name, struct addr* addr)
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

struct symbol* library_get_symbol(struct library* library, const char* name)
{
    int total = array_total(library->symbols);
    for (int i = 0; i < total; i++)
    {
        struct symbol* sym = array_get_index(library->symbols, i);
        if (strncmp(sym->name, name, sizeof(sym->name)) == 0)
        {
            return sym;
        }
    }

    return 0;
}