#ifndef cgame_ALLOC_H
#define cgame_ALLOC_H

#include <stddef.h>
#include "arena.h"
#include "stack.h"
#include "mem.h"

typedef struct
{
    size_t global;
    size_t stack;
} MemoryMetadata;

typedef struct
{
    ArenaMemory *global;
    StackMemory *stack;
    size_t usage;
} MemoryLayout;

extern MemoryLayout *alloc;

void alloc_create(MemoryMetadata meta);
void alloc_terminate();

#define xxstack(size) (stack_alloc(alloc->stack, size))
#define xxfreestack(ptr) (stack_free(alloc->stack, ptr))
#define xxarena(size) (arena_alloc(alloc->global, size))

#endif