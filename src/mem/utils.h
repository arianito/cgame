#ifndef cgame_UTILS_H
#define cgame_UTILS_H

#include <stddef.h>

#define MEM_DEBUG_MODE 0

#define PRINT_BITS(x)                              \
  do                                               \
  {                                                \
    typeof(x) a__ = (x);                           \
    char *p__ = (char *)&a__ + sizeof(x) - 1;      \
    size_t bytes__ = sizeof(x);                    \
    printf(#x ": ");                               \
    while (bytes__--)                              \
    {                                              \
      char bits__ = 8;                             \
      while (bits__--)                             \
        putchar(*p__ & (1 << bits__) ? '1' : '0'); \
      putchar('|');                                \
      p__--;                                       \
    }                                              \
    putchar('\n');                                 \
  } while (0)

#define FORMAT_BYTES(bt, buff, n)                                      \
  {                                                                    \
    const char suffix[5][3] = {"B\0", "KB\0", "MB\0", "GB\0", "TB\0"}; \
    int i = 0;                                                         \
    double bytes = (double)bt;                                         \
    while (bytes >= 1024 && i < 5)                                     \
    {                                                                  \
      bytes /= 1024;                                                   \
      i++;                                                             \
    }                                                                  \
    snprintf(buff, n, "%.2f %s", bytes, suffix[i]);                    \
  }

#define MODULO(address, alignment) (address & (alignment - 1UL))
#define NEXTPOW2(num) ((num < 2) ? 2 : ((num | (num >> 1) | (num >> 2) | (num >> 4) | (num >> 8) | (num >> 16)) + 1))
#define PREVPOW2(num) ((num <= 2) ? 2 : (((num | (num >> 1) | (num >> 2) | (num >> 4) | (num >> 8) | (num >> 16)) >> 1) + 1))
#define ISPOW2(alignment) (!(alignment & (alignment - 1UL)))
#define MEMORY_PADDING(address, alignment) ((alignment - (address & (alignment - 1UL))) & (alignment - 1UL))
#define MEMORY_PADDING_STD(address) (MEMORY_PADDING(address, sizeof(size_t)))
#define MEMORY_SPACE(space, alignment) ((space + alignment - 1UL) & ~(alignment - 1UL))
#define MEMORY_SPACE_STD(type) (MEMORY_SPACE(sizeof(type), sizeof(size_t)))
#define MEMORY_ALIGNMENT(address, space, alignment) (MEMORY_PADDING(address, alignment) + ((MEMORY_PADDING(address, alignment) >= space) ? 0UL : MEMORY_SPACE(space, alignment)))
#define MEMORY_ALIGNMENT_STD(address, type) (MEMORY_ALIGNMENT(address, sizeof(type), sizeof(size_t)))

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

#define BYTE6AB(n6, a, b) (SET_NTH_LE(n6, 6ULL, 0ULL) | SET_NTH_BE(b, 1ULL, 1ULL) | SET_NTH_BE(a, 1ULL, 0ULL))
#define BYTE6AB_SET_A(full, a) (full | SET_NTH_BE(a, 1ULL, 0ULL))
#define BYTE6AB_GET_A(full) (GET_NTH_BE(full, 1ULL, 0ULL))
#define BYTE6AB_SET_B(full, b) (full | SET_NTH_BE(a, 1ULL, 1ULL))
#define BYTE6AB_GET_B(full) (GET_NTH_BE(full, 1ULL, 1ULL))
#define BYTE6AB_GET_6(full) (GET_NTH_LE(full, 6ULL, 0ULL))

#define BYTES (1)
#define KILOBYTES (BYTES * 1024)
#define MEGABYTES (KILOBYTES * 1024)
#define GIGABYTES (MEGABYTES * 1024)

#endif