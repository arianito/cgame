#include "scalar.h"

#include <math.h>
#include <float.h>
#include <stdlib.h>
#include "defs.h"

// int

uint32_t floori(float a) { return (uint32_t)floorf(a); }

// float

float sqrf(float a)
{
    return sqrtf(a);
}

float floof(float a)
{
    return floorf(a);
}
float ceif(float a)
{
    return ceif(a);
}

float powerf(float a, float b)
{
    return powf(a, b);
}

bool validf(float a)
{
    if (isnan(a))
        return false;
    if (isinf(a))
        return false;
    return true;
}

float sindf(float d) { return sinf(d * DEG2RAD); }

float cosdf(float d) { return cosf(d * DEG2RAD); }

float tandf(float d) { return tanf(d * DEG2RAD); }

float tanhdf(float d) { return tanhf(d * DEG2RAD); }

float sinhdf(float d) { return sinhf(d * DEG2RAD); }

float coshdf(float d) { return coshf(d * DEG2RAD); }

float asindf(float a) { return asinf(a) * RAD2DEG; }

float acosdf(float a) { return acosf(a) * RAD2DEG; }

float atandf(float a) { return atanf(a) * RAD2DEG; }

float atan2df(float a, float b) { return atan2f(a, b) * RAD2DEG; }

float fractf(float a)
{
    return a - floorf(a);
}

float inv_sqrtf(float a)
{
    return 1.0f / sqrtf(a);
}

float moduluf(float a, float b)
{
    return fmodf(a, b);
}

float repeatf(float t, float length)
{
    return clampf(t - floorf(t / length) * length, 0.0f, length);
}

float ping_pongf(float t, float length)
{
    return length - fabsf(repeatf(t, length * 2.0f) - length);
}

float inv_lerpf(float a, float b, float value)
{
    return a != b ? clamp01f((value - a) / (b - a)) : 0.0f;
}

float slerpf(float from, float to, float t)
{
    float t0 = clamp01f(t);
    t0 = -2.0f * t0 * t0 * t0 + 3.0f * t0 * t0;
    return to * t0 + from * (1.0f - t0);
}

float delta_anglef(float current, float target)
{
    float delta = repeatf((target - current), 360.0f);
    return delta > 180.0f ? delta - 360.0f : delta;
}

float lerp_anglef(float a, float b, float t)
{
    float delta = repeatf((b - a), 360.0f);
    if (delta > 180)
        delta -= 360.0f;
    return a + delta * clamp01f(t);
}

float unwind_anglef(float a)
{
    while (a > 180.f)
        a -= 360.f;
    while (a < -180.f)
        a += 360.f;
    return a;
}

float clamp_axisf(float a)
{
    a = fmodf(a, 360.0f);
    if (a < 0.0f)
        a += 360.0f;
    return a;
}

float norm_axisf(float a)
{
    a = clamp_axisf(a);
    if (a > 180.0f)
        a -= 360.0f;
    return a;
}

float clamp_angle(float a, float min, float max)
{

    float md = clamp_axisf(max - min) * 0.5f;
    float rc = clamp_axisf(min + md);
    float dc = clamp_axisf(a - rc);

    if (dc > md)
        return norm_axisf(rc + md);
    else if (dc < -md)
        return norm_axisf(rc - md);

    return norm_axisf(a);
}

float move_towards_anglef(float current, float target, float maxDelta)
{
    float da = delta_anglef(current, target);
    if (-maxDelta < da && da < maxDelta)
        return target;
    return move_towardsf(current, current + da, maxDelta);
}

float smooth_stepf(float from, float to, float t)
{
    float t0 = clamp01f(t);
    t0 = -2.0F * t0 * t0 * t0 + 3.0F * t0 * t0;
    return to * t0 + from * (1.0F - t0);
}

float smooth_dampf(float current, float target, float *currentVelocity, float smoothTime, float maxSpeed, float delta)
{
    float smoothTime0 = maxf(0.0001F, smoothTime);
    float omega = 2.0F / smoothTime0;
    float x = omega * delta;
    float exp = 1.0F / (1.0F + x + 0.48F * x * x + 0.235F * x * x * x);
    float change = current - target;
    float originalTo = target;
    float maxChange = maxSpeed * smoothTime0;
    change = clampf(change, -maxChange, maxChange);
    float target0 = current - change;
    float temp = (*currentVelocity + omega * change) * delta;
    *currentVelocity = (*currentVelocity - omega * temp) * exp;
    float output = target0 + (change + temp) * exp;
    if (originalTo - current > 0.0F == output > originalTo)
    {
        output = originalTo;
        *currentVelocity = (output - originalTo) / delta;
    }
    return output;
}
float snapf(float a, float size)
{
    return (floorf((a + size * 0.5f) / size) * size);
}