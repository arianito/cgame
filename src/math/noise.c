#include "noise.h"

#include "math/scalar.h"

#include "math/defs.h"


uint32_t noise_1d(int PositionX, uint32_t Seed)
{
    const uint32_t SQ5_Bit_Noise1 = 0xd2a80a3f;
    const uint32_t SQ5_Bit_Noise2 = 0xa884f197;
    const uint32_t SQ5_Bit_Noise3 = 0x6C736F4B;
    const uint32_t SQ5_Bit_Noise4 = 0xB79F3ABB;
    const uint32_t SQ5_Bit_Noise5 = 0x1b56c4f5;
    uint32_t MangledBits = (uint32_t)(PositionX);
    MangledBits *= SQ5_Bit_Noise1;
    MangledBits += Seed;
    MangledBits ^= (MangledBits >> 9);
    MangledBits += SQ5_Bit_Noise2;
    MangledBits ^= (MangledBits >> 11);
    MangledBits *= SQ5_Bit_Noise3;
    MangledBits ^= (MangledBits >> 13);
    MangledBits += SQ5_Bit_Noise4;
    MangledBits ^= (MangledBits >> 15);
    MangledBits *= SQ5_Bit_Noise5;
    MangledBits ^= (MangledBits >> 17);
    return MangledBits;
}

uint32_t noise_2d(int PositionX, int PositionY, uint32_t Seed)
{
    const uint32_t Large_Prime = 198491317;
    return noise_1d(PositionX + (Large_Prime * PositionY), Seed);
}

uint32_t noise_3d(int PositionX, int PositionY, int PositionZ, uint32_t Seed)
{
    const uint32_t Large_Prime_1 = 198491317;
    const uint32_t Large_Prime_2 = 6542989;
    return noise_1d(
        PositionX +
            (Large_Prime_1 * PositionY) +
            (Large_Prime_2 * PositionZ),
        Seed);
}

uint32_t noise_4d(int PositionX, int PositionY, int PositionZ, int PositionW, uint32_t Seed)
{
    const uint32_t Large_Prime_1 = 198491317;
    const uint32_t Large_Prime_2 = 6542989;
    const uint32_t Large_Prime_3 = 357239;
    return noise_1d(
        PositionX +
            (Large_Prime_1 * PositionY) +
            (Large_Prime_2 * PositionZ) +
            (Large_Prime_3 * PositionW),
        Seed);
}

float noise_1d_zero_to_one(int PositionX, uint32_t Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0xFFFFFFFF));
    return One_Over_Max_Uint * (double)(noise_1d(PositionX, Seed));
}

float noise_2d_zero_to_one(int PositionX, int PositionY, uint32_t Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0xFFFFFFFF));
    return One_Over_Max_Uint * (double)(noise_2d(PositionX, PositionY, Seed));
}

float noise_3d_zero_to_one(int PositionX, int PositionY, int PositionZ, uint32_t Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0xFFFFFFFF));
    return One_Over_Max_Uint * (double)(noise_3d(PositionX, PositionY, PositionZ, Seed));
}

float noise_4d_zero_to_one(int PositionX, int PositionY, int PositionZ, int PositionW, uint32_t Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0xFFFFFFFF));
    return One_Over_Max_Uint * (double)(noise_4d(PositionX, PositionY, PositionZ, PositionW, Seed));
}

float noise_1d_neg_one_to_one(int PositionX, uint32_t Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0x7FFFFFFF));
    return One_Over_Max_Uint * (double)(noise_1d(PositionX, Seed));
}

float noise_2d_neg_one_to_one(int PositionX, int PositionY, uint32_t Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0x7FFFFFFF));
    return One_Over_Max_Uint * (double)(noise_2d(PositionX, PositionY, Seed));
}

float noise_3d_neg_one_to_one(int PositionX, int PositionY, int PositionZ, uint32_t Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0x7FFFFFFF));
    return One_Over_Max_Uint * (double)(noise_3d(PositionX, PositionY, PositionZ, Seed));
}

float noise_4d_neg_one_to_one(int PositionX, int PositionY, int PositionZ, int PositionW, uint32_t Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0x7FFFFFFF));
    return One_Over_Max_Uint * (double)(noise_4d(PositionX, PositionY, PositionZ, PositionW, Seed));
}

static float dot_grad_1d(int X, float X0, uint32_t Seed)
{
    const float T = noise_1d_neg_one_to_one(X, Seed);
    return ((X0 - X) * T);
}

static float dot_grad_2d(int X, int Y, float X0, float Y0, uint32_t Seed)
{
    const float T = noise_2d_neg_one_to_one(X, Y, Seed) * 360;
    return ((X0 - X) * cosdf(T)) + ((Y0 - Y) * sindf(T));
}

float noise_perlin_1d(float X, float Scale, uint32_t Seed)
{
    const float ScaleClamped = Scale <= 0 ? 1.0f : Scale;
    const float Xs = X / ScaleClamped;
    const int X0 = (int)floof(Xs);
    const int X1 = X0 + 1;
    const float Dx = fadef(Xs - X0);
    return lerpf(
        dot_grad_1d(X0, Xs, Seed),
        dot_grad_1d(X1, Xs, Seed), Dx);
}

float noise_perlin_2d(float X, float Y, float Scale, uint32_t Seed)
{
    const float ScaleClamped = Scale <= 0 ? 1.0f : Scale;
    const float Xs = X / ScaleClamped;
    const float Ys = Y / ScaleClamped;
    const int X0 = (int)floof(Xs);
    const int X1 = X0 + 1;
    const int Y0 = (int)floof(Ys);
    const int Y1 = Y0 + 1;
    const float Dx = fadef(Xs - X0);
    const float Dy = fadef(Ys - Y0);
    return lerpf(
        lerpf(
            dot_grad_2d(X0, Y0, Xs, Ys, Seed),
            dot_grad_2d(X1, Y0, Xs, Ys, Seed), Dx),
        lerpf(
            dot_grad_2d(X0, Y1, Xs, Ys, Seed),
            dot_grad_2d(X1, Y1, Xs, Ys, Seed), Dx),
        Dy);
}
