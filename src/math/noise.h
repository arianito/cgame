#ifndef cgame_NOISE_H
#define cgame_NOISE_H

#include <stdint.h>

uint32_t noise_1d(int PositionX, uint32_t Seed);
uint32_t noise_2d(int PositionX, int PositionY, uint32_t Seed);
uint32_t noise_3d(int PositionX, int PositionY, int PositionZ, uint32_t Seed);
uint32_t noise_4d(int PositionX, int PositionY, int PositionZ, int PositionW, uint32_t Seed);
float noise_1d_zero_to_one(int PositionX, uint32_t Seed);
float noise_2d_zero_to_one(int PositionX, int PositionY, uint32_t Seed);
float noise_3d_zero_to_one(int PositionX, int PositionY, int PositionZ, uint32_t Seed);
float noise_4d_zero_to_one(int PositionX, int PositionY, int PositionZ, int PositionW, uint32_t Seed);
float noise_1d_neg_one_to_one(int PositionX, uint32_t Seed);
float noise_2d_neg_one_to_one(int PositionX, int PositionY, uint32_t Seed);
float noise_3d_neg_one_to_one(int PositionX, int PositionY, int PositionZ, uint32_t Seed);
float noise_4d_neg_one_to_one(int PositionX, int PositionY, int PositionZ, int PositionW, uint32_t Seed);
float noise_perlin_1d(float X, float Scale, uint32_t Seed);
float noise_perlin_2d(float X, float Y, float Scale, uint32_t Seed);

#endif