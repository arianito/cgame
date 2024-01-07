#ifndef cgame_UTILS_H
#define cgame_UTILS_H

#include <stddef.h>
#include "defs.h"

static inline size_t MEMORY_PADDING(size_t address)
{
  size_t modulo = address & (DEFAULT_MEMORY_ALIGNMENT - 1);
  size_t padding = 0;

  if (modulo != 0)
    padding = DEFAULT_MEMORY_ALIGNMENT - modulo;

  return padding;
}

static inline size_t MEMORY_SPACE(size_t space)
{
  int n = (space & (DEFAULT_MEMORY_ALIGNMENT - 1)) > 0 ? 1 : 0;
  return DEFAULT_MEMORY_ALIGNMENT * ((space / DEFAULT_MEMORY_ALIGNMENT) + n);
}

static inline size_t MEMORY_ALIGNMENT(size_t address, size_t space)
{
  size_t modulo = address & (DEFAULT_MEMORY_ALIGNMENT - 1);
  size_t padding = 0;

  if (modulo != 0)
    padding = DEFAULT_MEMORY_ALIGNMENT - modulo;

  if (padding >= space)
    return padding;

  int n = (space & (DEFAULT_MEMORY_ALIGNMENT - 1)) > 0 ? 1 : 0;
  return padding + DEFAULT_MEMORY_ALIGNMENT * ((space / DEFAULT_MEMORY_ALIGNMENT) + n);
}

#define BYTE_BE(offset) ((sizeof(size_t) - (size_t)(offset + 1ULL)) << 3ULL)
#define BYTE_LE(offset) ((size_t)(offset) << 3ULL)
#define MASK_BE(offset) ((1ULL << BYTE_BE(offset - 1ULL)) - 1ULL)
#define MASK_LE(offset) ((1ULL << BYTE_LE(offset)) - 1ULL)
#define SET_NTH_BE(number, bytes, offset) (((number & MASK_LE(bytes)) << BYTE_BE(offset)))
#define SET_NTH_LE(number, bytes, offset) (((number & MASK_LE(bytes)) << BYTE_LE(offset)))
#define GET_NTH_BE(number, bytes, offset) (((size_t)number >> BYTE_BE(offset)) & MASK_LE(bytes))
#define GET_NTH_LE(number, bytes, offset) (((size_t)number >> BYTE_LE(offset)) & MASK_LE(bytes))

#define BYTE71(n7, n1) (SET_NTH_LE(n7, 7ULL, 0ULL) | SET_NTH_BE(n1, 1ULL, 0ULL))
#define BYTE71_GET_7(full) (GET_NTH_LE(full, 7ULL, 0ULL))
#define BYTE71_GET_1(full) (GET_NTH_BE(full, 1ULL, 0ULL))
#define BYTE71_SET_7(full, n7) (full | SET_NTH_LE(n7, 7ULL, 0ULL))
#define BYTE71_SET_1(full, n1) (full | SET_NTH_BE(n1, 1ULL, 0ULL))

#endif