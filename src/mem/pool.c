
#include "pool.h"

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

void pool_enqueue(PoolMemory *self, PoolMemoryNode *node)
{
    node->next = BYTE71((size_t)self->head, 0);
    self->head = node;
}

PoolMemoryNode *pool_dequeue(PoolMemory *self)
{
    if (self->head == NULL)
        return NULL;
    PoolMemoryNode *node = self->head;
    node->next = BYTE71_SET_1(node->next, 1);
    self->head = (PoolMemoryNode *)(BYTE71_GET_7(node->next));
    return node;
}

void *pool_alloc(PoolMemory *self)
{
    PoolMemoryNode *node = pool_dequeue(self);
    self->usage -= self->object_size;
    const size_t space = MEMORY_SPACE_STD(PoolMemoryNode);
    return (void *)((size_t)node + space);
}

void pool_free(PoolMemory *self, void *ptr)
{
    if (!ptr)
        return;
    size_t address = (size_t)ptr;
    const size_t space = MEMORY_SPACE_STD(PoolMemoryNode);
    PoolMemoryNode *node = (PoolMemoryNode *)(address - space);
    size_t used = BYTE71_GET_1(node->next);
    if (!used)
        return;
    pool_enqueue(self, node);
    self->usage += self->object_size;
}

void pool_destroy(PoolMemory *self)
{
    size_t op = (size_t)self - self->padding;
    free((void *)(op));
}

PoolMemory *pool_create(void *m, size_t size, size_t objectSize)
{
    size_t start = (size_t)m;
    const size_t space = MEMORY_SPACE_STD(PoolMemory);
    const size_t pool_space = MEMORY_SPACE_STD(PoolMemoryNode);
    const size_t padding = MEMORY_PADDING_STD(start);
    PoolMemory *self = (PoolMemory *)(start + padding);
    self->head = NULL;
    self->total = size;
    self->padding = padding;
    self->object_size = objectSize;
    self->usage = 0;
    size_t cursor = padding + space;
    while (1)
    {
        size_t address = start + cursor;
        const size_t padding = MEMORY_ALIGNMENT_STD(address, PoolMemoryNode);
        cursor += padding + objectSize;
        if (cursor > size)
            break;
        pool_enqueue(self, (PoolMemoryNode *)(address + padding - pool_space));
        self->usage += self->object_size;
    }
    return self;
}

PoolMemory *make_pool(size_t size, size_t objectSize)
{
    void *m = malloc(size);
    if (!m)
        return;
    return pool_create(m, size, objectSize);
}

size_t pool_size(size_t size, size_t objectSize)
{
    const size_t n = size / objectSize;
    size += MEMORY_SPACE_STD(PoolMemory) + sizeof(size_t);
    size += n * sizeof(PoolMemoryNode);
    return size;
}