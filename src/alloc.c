
#include <jemalloc/jemalloc.h>
#include <stdint.h>
#include <memory.h>
#include "mem/alloc.h"
#include "mem/defs.h"

void *xxmalloc(size_t size)
{
    alloc->usage += size;
    return aligned_alloc(DEFAULT_MEMORY_ALIGNMENT, size);
}

extern void xxfree(void *ptr, size_t size)
{
    alloc->usage -= size;
    free(ptr);
}
