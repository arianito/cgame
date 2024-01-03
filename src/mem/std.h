#ifndef cgame_STD_H
#define cgame_STD_H

#include <stddef.h>
#include <stdint.h>

void *std_alloc(size_t size, uint32_t alignment);
void std_free(void *ptr);

#endif
