
#include "stack.h"

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

typedef struct
{
    size_t padding;
} StackMemoryNode;

void *stack_alloc(StackMemory *self, size_t size, size_t alignment)
{
    size_t start = (size_t)self - self->padding;
    size_t address = start + self->usage;
    const size_t padding = MEMORY_ALIGNMENT(address, sizeof(StackMemoryNode), alignment);
    const size_t space = MEMORY_SPACE(sizeof(StackMemoryNode), sizeof(size_t));
    if (self->usage + padding + size > self->total)
        return NULL;
    self->usage += padding + size;
    StackMemoryNode *node = (StackMemoryNode *)(address + padding - space);
    node->padding = padding;
    return (void *)(address + padding);
}

void stack_free(StackMemory *self, void *ptr)
{
    if (!ptr)
        return;
    size_t start = (size_t)self - self->padding;
    const size_t space = MEMORY_SPACE_STD(StackMemoryNode);
    StackMemoryNode *node = (StackMemoryNode *)((size_t)ptr - space);
    self->usage = ((size_t)ptr - node->padding) - start;
}

void* stack_realloc(StackMemory *self, void *ptr, size_t new_size)
{
    if (!ptr)
        return;
    size_t start = (size_t)self - self->padding;
    const size_t space = MEMORY_SPACE(sizeof(StackMemoryNode), sizeof(size_t));
    StackMemoryNode *node = (StackMemoryNode *)((size_t)ptr - space);
    size_t usage = ((size_t)ptr) - start + new_size;
    if (usage > self->total)
        return;
    self->usage = usage;
    return ptr;
}
void stack_reset(StackMemory *self)
{
    const size_t space = MEMORY_SPACE_STD(StackMemory);
    self->usage = self->padding + space;
}

void stack_destroy(StackMemory *self)
{
    size_t op = (size_t)self - self->padding;
    free((void *)(op));
}

StackMemory *stack_create(void *m, size_t size)
{
    size_t address = (size_t)m;
    const size_t space = MEMORY_SPACE_STD(StackMemory);
    const size_t padding = MEMORY_PADDING_STD(address);
    StackMemory *self = (StackMemory *)(address + padding);
    self->total = size;
    self->usage = padding + space;
    self->padding = padding;
    return self;
}

StackMemory *make_stack(size_t size)
{
    void *m = malloc(size);
    if (!m)
        return NULL;
    return stack_create(m, size);
}