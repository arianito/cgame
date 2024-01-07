#ifndef cgame_MEM_H
#define cgame_MEM_H

#include <stddef.h>

extern void *xxmalloc(size_t size);

extern void xxfree(void *ptr, size_t size);

#endif