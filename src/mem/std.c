
#include "std.h"

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

void *std_alloc(size_t size, uint32_t alignment)
{
    void *ptr = malloc(size + alignment);
    if (!ptr)
        return NULL;
    size_t address = (size_t)ptr;
    char padding = MEMORY_ALIGNMENT(address, 1, alignment);
    address += padding;
    *((char *)(address - 1)) = padding;
    return (void *)address;
}

void std_free(void *ptr)
{
    if (!ptr)
        return;
    size_t address = (size_t)(ptr);
    free((void *)(address - *((char *)(address - 1))));
}