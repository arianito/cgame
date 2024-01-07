
#include "alloc.h"

#include <malloc.h>

MemoryLayout *alloc = NULL;

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
