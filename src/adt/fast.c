#include "fast.h"
#include <immintrin.h>

uint16_t __fast_match_empty(__Byte16 ctrl)
{
    return (uint16_t)_mm_movemask_epi8(ctrl);
}

uint16_t __fast_match(__Byte16 ctrl, int8_t hash)
{
    return (uint16_t)_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_set1_epi8(hash), ctrl));
}

void __fast_set_byte(__Byte16 *current, uint8_t value, uint8_t index)
{
    uint8_t *to = (uint8_t *)(current);
    to[index] = value;
}

__Byte16 __fast_set1_epi8(char c)
{
    return _mm_set1_epi8(c);
}