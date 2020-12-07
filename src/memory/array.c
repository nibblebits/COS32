#include "array.h"
#include "memory/kheap.h"
#include "memory/memory.h"

struct array *array_create(size_t element_size)
{
    struct array *array = kzalloc(sizeof(struct array));
    array->e_sz = element_size;
    return array;
}

void array_insert(struct array *array, void *ptr)
{
    void *old_ptr = array->ptr;
    void *new_ptr = kzalloc(array->e_sz * (array->total + 1));
    // Points to the last element in the new_ptr array.
    void *new_last_element = new_ptr + (array->e_sz * array->total);

    // If we had some array elements before this then copy it to the new array
    if (old_ptr)
    {
        memcpy(new_ptr, old_ptr, array->e_sz * array->total);
    }
    memcpy(new_last_element, ptr, array->e_sz);
}

void *array_get_index(struct array *array, int index)
{
    return array->ptr + (array->e_sz * index);
}

size_t array_total(struct array* array)
{
    return array->total;
}

void array_destroy(struct array *array)
{
    kfree(array->ptr);
    kfree(array);
}