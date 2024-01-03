#ifndef cgame_STACK_H
#define cgame_STACK_H

#include <stddef.h>

typedef struct
{
    size_t padding;
    size_t total;
    size_t usage;
} StackMemory;

StackMemory *stack_create(void *m, size_t size);

StackMemory *make_stack(size_t size);

void stack_reset(StackMemory *self);

void stack_destroy(StackMemory *self);

void *stack_alloc(StackMemory *self, size_t size, size_t alignment);

void *stack_realloc(StackMemory *self, void *ptr, size_t new_size);

void stack_free(StackMemory *self, void *ptr);

#endif