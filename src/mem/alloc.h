#ifndef cgame_ALLOC_H
#define cgame_ALLOC_H

#include <jemalloc/jemalloc.h>
#include <stddef.h>
#include <stdio.h>
#include "arena.h"
#include "stack.h"
#include "utils.h"

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

void *xxmalloc(size_t size);

void *xxrealloc(void *ptr, size_t size);

void xxfree(void *ptr);

#endif