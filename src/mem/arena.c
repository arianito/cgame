
#include "arena.h"

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "mem.h"

void *arena_alloc(ArenaMemory *self, size_t size)
{
    size_t address = ((size_t)self - self->padding) + self->usage;
    const size_t padding = MEMORY_PADDING(address);
    if (self->usage + size + padding > self->total)
    {
        printf("arena: out of memory %zu\n");
        return NULL;
    }
    address += padding;
    self->usage += size + padding;
    return (void *)(address);
}

void arena_reset(ArenaMemory *self)
{
    const size_t space = MEMORY_SPACE(sizeof(ArenaMemory));
    self->usage = self->padding + space;
}

void arena_destroy(ArenaMemory *self)
{
    size_t op = (size_t)self - self->padding;
    xxfree((void *)(op), self->total);
}

ArenaMemory *make_arena_raw(void *m, size_t size)
{
    size_t address = (size_t)m;
    const size_t space = MEMORY_SPACE(sizeof(ArenaMemory));
    const size_t padding = MEMORY_PADDING(address);
    ArenaMemory *self = (ArenaMemory *)(address + padding);
    self->total = size;
    self->usage = padding + space;
    self->padding = padding;
    return self;
}

ArenaMemory *make_arena(size_t size)
{
    void *m = xxmalloc(size);
    if (!m)
    {
        printf("arena: malloc failed %zu\n");
        exit(1);
        return NULL;
    }
    return make_arena_raw(m, size);
}
