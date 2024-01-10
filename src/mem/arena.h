#ifndef cgame_ARENA_H
#define cgame_ARENA_H

#include <stddef.h>

typedef struct  {
    size_t padding;
    size_t total;
    size_t usage;
} ArenaMemory;

ArenaMemory *make_arena(size_t size);

ArenaMemory *make_arena_raw(void *m, size_t size);

void arena_destroy(ArenaMemory *self);

void *arena_alloc(ArenaMemory *self, size_t size);

void arena_reset(ArenaMemory *self);

#endif