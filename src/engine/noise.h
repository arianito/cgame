#ifndef cgame_NOISE_H
#define cgame_NOISE_H

#include "mathf.h"

inline static unsigned int noise_1d(int PositionX, unsigned int Seed)
{
    const unsigned int SQ5_Bit_Noise1 = 0xd2a80a3f;
    const unsigned int SQ5_Bit_Noise2 = 0xa884f197;
    const unsigned int SQ5_Bit_Noise3 = 0x6C736F4B;
    const unsigned int SQ5_Bit_Noise4 = 0xB79F3ABB;
    const unsigned int SQ5_Bit_Noise5 = 0x1b56c4f5;
    unsigned int MangledBits = (unsigned int)(PositionX);
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

inline static unsigned int noise_2d(int PositionX, int PositionY, unsigned int Seed)
{
    const unsigned int Large_Prime = 198491317;
    return noise_1d(PositionX + (Large_Prime * PositionY), Seed);
}

inline static unsigned int noise_3d(int PositionX, int PositionY, int PositionZ, unsigned int Seed)
{
    const unsigned int Large_Prime_1 = 198491317;
    const unsigned int Large_Prime_2 = 6542989;
    return noise_1d(
        PositionX +
            (Large_Prime_1 * PositionY) +
            (Large_Prime_2 * PositionZ),
        Seed);
}

inline static unsigned int noise_4d(int PositionX, int PositionY, int PositionZ, int PositionW, unsigned int Seed)
{
    const unsigned int Large_Prime_1 = 198491317;
    const unsigned int Large_Prime_2 = 6542989;
    const unsigned int Large_Prime_3 = 357239;
    return noise_1d(
        PositionX +
            (Large_Prime_1 * PositionY) +
            (Large_Prime_2 * PositionZ) +
            (Large_Prime_3 * PositionW),
        Seed);
}

inline static float noise_1d_zero_to_one(int PositionX, unsigned int Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0xFFFFFFFF));
    return One_Over_Max_Uint * (double)(noise_1d(PositionX, Seed));
}

inline static float noise_2d_zero_to_one(int PositionX, int PositionY, unsigned int Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0xFFFFFFFF));
    return One_Over_Max_Uint * (double)(noise_2d(PositionX, PositionY, Seed));
}

inline static float noise_3d_zero_to_one(int PositionX, int PositionY, int PositionZ, unsigned int Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0xFFFFFFFF));
    return One_Over_Max_Uint * (double)(noise_3d(PositionX, PositionY, PositionZ, Seed));
}

inline static float noise_4d_zero_to_one(int PositionX, int PositionY, int PositionZ, int PositionW, unsigned int Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0xFFFFFFFF));
    return One_Over_Max_Uint * (double)(noise_4d(PositionX, PositionY, PositionZ, PositionW, Seed));
}

inline static float noise_1d_neg_one_to_one(int PositionX, unsigned int Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0x7FFFFFFF));
    return One_Over_Max_Uint * (double)(noise_1d(PositionX, Seed));
}

inline static float noise_2d_neg_one_to_one(int PositionX, int PositionY, unsigned int Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0x7FFFFFFF));
    return One_Over_Max_Uint * (double)(noise_2d(PositionX, PositionY, Seed));
}

inline static float noise_3d_neg_one_to_one(int PositionX, int PositionY, int PositionZ, unsigned int Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0x7FFFFFFF));
    return One_Over_Max_Uint * (double)(noise_3d(PositionX, PositionY, PositionZ, Seed));
}

inline static float noise_4d_neg_one_to_one(int PositionX, int PositionY, int PositionZ, int PositionW, unsigned int Seed)
{
    const double One_Over_Max_Uint = (1.0 / (double)(0x7FFFFFFF));
    return One_Over_Max_Uint * (double)(noise_4d(PositionX, PositionY, PositionZ, PositionW, Seed));
}

inline static float dot_grad_1d(int X, float X0, unsigned int Seed)
{
    const float T = noise_1d_neg_one_to_one(X, Seed);
    return ((X0 - X) * T);
}

inline static float dot_grad_2d(int X, int Y, float X0, float Y0, unsigned int Seed)
{
    const float T = noise_2d_neg_one_to_one(X, Y, Seed) * PI * 2;
    return ((X0 - X) * cosf(T)) + ((Y0 - Y) * sinf(T));
}

inline static float fade(float T)
{
    return ((6 * T - 15) * T + 10) * T * T * T;
}

inline static float noise_perlin_1d(float X, float Scale, unsigned int Seed)
{
    const float ScaleClamped = Scale <= 0 ? 1.0f : Scale;
    const float Xs = X / ScaleClamped;
    const int X0 = (int)floorf(Xs);
    const int X1 = X0 + 1;
    const float Dx = fade(Xs - X0);
    return lerp(
        dot_grad_1d(X0, Xs, Seed),
        dot_grad_1d(X1, Xs, Seed), Dx);
}

inline static float noise_perlin_2d(float X, float Y, float Scale, unsigned int Seed)
{
    const float ScaleClamped = Scale <= 0 ? 1.0f : Scale;
    const float Xs = X / ScaleClamped;
    const float Ys = Y / ScaleClamped;
    const int X0 = (int)floorf(Xs);
    const int X1 = X0 + 1;
    const int Y0 = (int)floorf(Ys);
    const int Y1 = Y0 + 1;
    const float Dx = fade(Xs - X0);
    const float Dy = fade(Ys - Y0);
    return lerp(
        lerp(
            dot_grad_2d(X0, Y0, Xs, Ys, Seed),
            dot_grad_2d(X1, Y0, Xs, Ys, Seed), Dx),
        lerp(
            dot_grad_2d(X0, Y1, Xs, Ys, Seed),
            dot_grad_2d(X1, Y1, Xs, Ys, Seed), Dx),
        Dy);
}

#endif