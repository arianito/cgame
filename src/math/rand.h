#ifndef cgame_RAND_H
#define cgame_RAND_H

#include <stdint.h>

void seedi(uint32_t seed);
uint32_t randi();
void seedf(uint32_t seed);
float randf();

#endif