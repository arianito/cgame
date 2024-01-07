#ifndef cgame_POOL_H
#define cgame_POOL_H

#include <stddef.h>

typedef struct  {
    struct PoolMemoryNode *head;
    size_t padding;
    size_t object_size;
    size_t total;
    size_t usage;
} PoolMemory;

PoolMemory *make_pool(size_t size, size_t objectSize);

PoolMemory *pool_create(void *m, size_t size, size_t objectSize);

size_t pool_size(size_t size, size_t objectSize);

void pool_destroy(PoolMemory *self);

void *pool_alloc(PoolMemory *self);

void pool_free(PoolMemory *self, void *ptr);

#endif