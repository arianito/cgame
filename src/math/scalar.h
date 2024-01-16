#ifndef cgame_SCALAR_H
#define cgame_SCALAR_H

#include <stdint.h>
#include <stdbool.h>

#define minf(a, b) ((a) > (b) ? (b) : (a))
#define maxf(a, b) ((a) > (b) ? (a) : (b))
#define signf(a) ((a) >= 0 ? 1.0 : -1.0)
#define squaref(a) ((a) * (a))
#define selectf(a, b, c) ((a) >= 0 ? (b) : (c))
#define rampf(a) ((a) < 0 ? 0 : (a))
#define absf(a) ((a) > 0 ? (a) : -(a))

#define clampf(a, min, max) (minf(maxf((a), (min)), (max)))
#define clamp01f(a) (clampf((a), 0, 1))
#define lerp01f(a, b, t) ((a) + ((b) - (a)) * clamp01f(t))
#define lerpf(a, b, t) ((a) + ((b) - (a)) * (t))
#define near_eqf(a, b) (absf((b) - (a)) <= EPSILON)
#define near0f(a) (near_eqf((a), 0))
#define fadef(T) (((6 * (T)-15) * (T) + 10) * (T) * (T) * (T))
#define move_towardsf(current, target, maxDelta) (absf((target) - (current)) <= (maxDelta) ? (target) : ((current) + signf((target) - (current)) * (maxDelta)))

uint32_t floori(float a);

float sqrf(float a);
float powerf(float a, float b);
float floof(float a);
float ceif(float a);

bool validf(float a);
float sindf(float d);
float cosdf(float d);
float tandf(float d);
float tanhdf(float d);
float sinhdf(float d);
float coshdf(float d);
float asindf(float a);
float acosdf(float a);
float atandf(float a);
float atan2df(float a, float b);
float fractf(float a);
float inv_sqrtf(float a);
float moduluf(float a, float b);
float repeatf(float t, float length);
float ping_pongf(float t, float length);
float inv_lerpf(float a, float b, float value);
float slerpf(float from, float to, float t);
float delta_anglef(float current, float target);
float lerp_anglef(float a, float b, float t);
float unwind_anglef(float a);
float clamp_axisf(float a);
float norm_axisf(float a);
float clamp_angle(float a, float min, float max);
float move_towards_anglef(float current, float target, float maxDelta);
float smooth_stepf(float from, float to, float t);
float smooth_dampf(float current, float target, float *currentVelocity, float smoothTime, float maxSpeed, float delta);
float snapf(float a, float size);
float lg2f(float a);
float lg10f(float a);
inline static uint8_t log2i(uint32_t value)
{
    static const uint8_t table[64] =
        {
            63, 0, 58, 1, 59, 47, 53, 2,
            60, 39, 48, 27, 54, 33, 42, 3,
            61, 51, 37, 40, 49, 18, 28, 20,
            55, 30, 34, 11, 43, 14, 22, 4,
            62, 57, 46, 52, 38, 26, 32, 41,
            50, 36, 17, 19, 29, 10, 13, 21,
            56, 45, 25, 31, 35, 16, 9, 12,
            44, 24, 15, 8, 23, 7, 6, 5};
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return table[((value - (value >> 1)) * 0x07EDD5E59A4E28C2) >> 58];
}

#endif