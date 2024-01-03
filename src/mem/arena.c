
#include "arena.h"

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

void *arena_alloc(ArenaMemory *self, size_t size, size_t alignment)
{
    size_t address = ((size_t)self - self->padding) + self->usage;
    const size_t padding = MEMORY_PADDING(address, alignment);
    if (self->usage + size + padding > self->total)
        return NULL;
    address += padding;
    self->usage += size + padding;
    return (void *)(address);
}

void arena_reset(ArenaMemory *self)
{
    const size_t space = MEMORY_SPACE_STD(ArenaMemory);
    self->usage = self->padding + space;
}

void arena_destroy(ArenaMemory *self)
{
    size_t op = (size_t)self - self->padding;
    free((void *)(op));
}

ArenaMemory *arena_create(void *m, size_t size)
{
    size_t address = (size_t)m;
    const size_t space = MEMORY_SPACE_STD(ArenaMemory);
    const size_t padding = MEMORY_PADDING_STD(address);
    ArenaMemory *self = (ArenaMemory *)(address + padding);
    self->total = size;
    self->usage = padding + space;
    self->padding = padding;
    return self;
}

ArenaMemory *make_arena(size_t size)
{
    void *m = malloc(size);
    if (!m)
        return NULL;
    return arena_create(m, size);
}
