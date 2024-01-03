
#include "alloc.h"

#include "std.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

MemoryLayout *alloc = NULL;

void *xxmalloc(size_t size)
{
    void *ptr = malloc(size);
    alloc->usage += malloc_usable_size(ptr);
    return ptr;
}

void *xxrealloc(void *ptr, size_t size)
{
    if (ptr != NULL)
    {
        alloc->usage -= malloc_usable_size(ptr);
    }
    alloc->usage += size;
    return realloc(ptr, size);
}

void xxfree(void *ptr)
{
    if (ptr != NULL)
    {
        alloc->usage -= malloc_usable_size(ptr);
    }
    free(ptr);
}

void alloc_create(MemoryMetadata meta)
{
    alloc = malloc(sizeof(MemoryLayout));
    alloc->global = make_arena(meta.global);
    alloc->stack = make_stack(meta.stack);
}

void alloc_terminate()
{
    stack_destroy(alloc->stack);
    arena_destroy(alloc->global);
    free(alloc);
}
