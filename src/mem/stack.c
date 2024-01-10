
#include "stack.h"

#include <stdlib.h>
#include "utils.h"
#include "mem.h"

typedef struct
{
    size_t padding;
} StackMemoryNode;

void *stack_alloc(StackMemory *self, size_t size)
{
    size_t start = (size_t)self - self->padding;
    size_t address = start + self->usage;
    const size_t padding = MEMORY_ALIGNMENT(address, sizeof(StackMemoryNode));
    const size_t space = MEMORY_SPACE(sizeof(StackMemoryNode));
    if (self->usage + padding + size > self->total)
    {
        printf("stack: out of memory %zu\n");
        return NULL;
    }
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
    const size_t space = MEMORY_SPACE(sizeof(StackMemoryNode));
    StackMemoryNode *node = (StackMemoryNode *)((size_t)ptr - space);
    self->usage = ((size_t)ptr - node->padding) - start;
}

void stack_reset(StackMemory *self)
{
    const size_t space = MEMORY_SPACE(sizeof(StackMemory));
    self->usage = self->padding + space;
}

void stack_destroy(StackMemory *self)
{
    size_t op = (size_t)self - self->padding;
    xxfree((void *)(op), self->total);
}

StackMemory *make_stack_raw(void *m, size_t size)
{
    size_t address = (size_t)m;
    const size_t space = MEMORY_SPACE(sizeof(StackMemory));
    const size_t padding = MEMORY_PADDING(address);
    StackMemory *self = (StackMemory *)(address + padding);
    self->total = size;
    self->usage = padding + space;
    self->padding = padding;
    return self;
}

StackMemory *make_stack(size_t size)
{
    void *m = xxmalloc(size);
    if (!m)
    {
        printf("stack: malloc failed %zu\n");
        exit(1);
        return NULL;
    }
    return make_stack_raw(m, size);
}