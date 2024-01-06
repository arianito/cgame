#include "rand.h"

#include <stdlib.h>

#define IMAX_BITS(m) ((m) / ((m) % 255 + 1) / 255 % 255 * 8 + 7 - 86 / ((m) % 255 + 12))
#define RAND_MAX_WIDTH IMAX_BITS(RAND_MAX)

void seedi(uint32_t seed)
{
    srand(seed);
}

uint32_t randi()
{
    uint32_t r = 0;
    for (uint32_t i = 0; i < 32; i += RAND_MAX_WIDTH)
    {
        r <<= RAND_MAX_WIDTH;
        r ^= (uint32_t)rand();
    }
    return r;
}
void seedf(uint32_t seed)
{
    srand(seed);
}

float randf()
{
    uint32_t r = 0;
    for (uint32_t i = 0; i < 32; i += RAND_MAX_WIDTH)
    {
        r <<= RAND_MAX_WIDTH;
        r ^= (uint32_t)rand();
    }
    return (float)r / (float)0xffffffff;
}