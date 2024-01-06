#ifndef cgame_VEC2_H
#define cgame_VEC2_H

#include "vec2_ifc.h"
#include "vec3_ifc.h"
#include <stdbool.h>
#include "defs.h"
#include "scalar.h"

inline static Vec2 vec2yz(Vec3 a) { return vec2(a.y, a.z); }
inline static Vec2 vec2xy(Vec3 a) { return vec2(a.x, a.y); }
inline static Vec2 vec2xz(Vec3 a) { return vec2(a.x, a.z); }

inline static Vec2 vec2_neg(Vec2 a) { return vec2(-a.x, -a.y); }
inline static Vec2 vec2_add(Vec2 a, Vec2 b) { return vec2(a.x + b.x, a.y + b.y); }
inline static Vec2 vec2_addf(Vec2 a, float b) { return vec2(a.x + b, a.y + b); }
inline static Vec2 vec2_sub(Vec2 a, Vec2 b) { return vec2(a.x - b.x, a.y - b.y); }
inline static Vec2 vec2_subf(Vec2 a, float b) { return vec2(a.x - b, a.y - b); }
inline static Vec2 vec2_mul(Vec2 a, Vec2 b) { return vec2(a.x * b.x, a.y * b.y); }
inline static Vec2 vec2_mulvf(Vec2 a, float b) { return vec2(a.x * b, a.y * b); }
inline static Vec2 vec2_mulfv(float b, Vec2 a) { return vec2(a.x * b, a.y * b); }
/// a + s * b
static inline Vec2 vec2_mul_add(Vec2 a, float s, Vec2 b) { return vec2(a.x + s * b.x, a.y + s * b.y); }
static inline Vec2 vec2_mul_sub(Vec2 a, float s, Vec2 b) { return vec2(a.x - s * b.x, a.y - s * b.y); }
inline static Vec2 vec2_divf(Vec2 a, float b) { return near0f(b) ? vec2_inf : vec2(a.x / b, a.y / b); }
inline static Vec2 vec2_rotate(Vec2 a, float d) { return vec2(a.x * sindf(d) + a.y * cosdf(d), a.x * cosdf(d) - a.y * sindf(d)); }
inline static float vec2_dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }
inline static float vec2_sqr_length(Vec2 a) { return vec2_dot(a, a); }
inline static Vec2 vec2_norm(Vec2 a)
{
    float b = vec2_sqr_length(a);
    if (b == 1.0f)
        return a;
    if (b <= EPSILON2)
        return vec2_zero;

    float si = inv_sqrtf(b);
    a.x *= si;
    a.y *= si;
    return a;
}
inline static float vec2_angle(Vec2 a, Vec2 b)
{
    float de = vec2_sqr_length(a) * vec2_sqr_length(b);
    if (de <= EPSILON2)
        return 0.0f;
    return acosdf(clampf(vec2_dot(a, b) * inv_sqrtf(de), -1.0f, 1.0f));
}

inline static float vec2_sqr_distance(Vec2 a, Vec2 b) { return vec2_sqr_length(vec2_sub(a, b)); }
inline static float vec2_length(Vec2 a) { return sqrf(vec2_dot(a, a)); }
inline static float vec2_distance(Vec2 a, Vec2 b) { return vec2_length(vec2_sub(a, b)); }

inline static Vec2 vec2_length_normal(float *length, Vec2 v)
{
    *length = vec2_length(v);
    if ((*length) == 1)
        return v;
    if ((*length) < EPSILON)
        return vec2_zero;
    float invLength = 1.0f / *length;
    return vec2(invLength * v.x, invLength * v.y);
}
inline static bool vec2_eq(Vec2 a, Vec2 b) { return a.x == b.x && a.y == b.y; }
inline static bool vec2_eq0(Vec2 a) { return a.x == 0.0f && a.y == 0.0f; }
inline static bool vec2_near_eq(Vec2 a, Vec2 b) { return vec2_distance(a, b) < EPSILON; }
inline static bool vec2_near0(Vec2 a) { return vec2_near_eq(a, vec2_zero); }
inline static Vec2 vec2_min(Vec2 a, Vec2 b) { return vec2(minf(a.x, b.x), minf(a.y, b.y)); }
inline static float vec2_min_comp(Vec2 a) { return minf(a.x, a.y); }
inline static Vec2 vec2_max(Vec2 a, Vec2 b) { return vec2(maxf(a.x, b.x), maxf(a.y, b.y)); }
inline static float vec2_max_comp(Vec2 a) { return maxf(a.x, a.y); }
inline static Vec2 vec2_abs(Vec2 a) { return vec2(absf(a.x), absf(a.y)); }
inline static float vec2_abs_min(Vec2 a) { return minf(absf(a.x), absf(a.y)); }
inline static float vec2_abs_max(Vec2 a) { return maxf(absf(a.x), absf(a.y)); }
inline static Vec2 vec2_perp_cw(Vec2 v) { return vec2(v.y, -v.x); }
inline static Vec2 vec2_perp_ccw(Vec2 v) { return vec2(-v.y, v.x); }
inline static float vec2_cos_angle(Vec2 a, Vec2 b) { return vec2_dot(vec2_norm(a), vec2_norm(b)); }
inline static Vec2 vec2_sign(Vec2 a) { return vec2(signf(a.x), signf(a.y)); }
inline static Vec2 vec2_lerp(Vec2 a, Vec2 b, float dt) { return vec2(a.x + (b.x - a.x) * dt, a.y + (b.y - a.y) * dt); }
inline static Vec2 vec2_lerp01(Vec2 a, Vec2 b, float dt) { return vec2_lerp(a, b, clamp01f(dt)); }
inline static Vec2 vec2_reflect(Vec2 a, Vec2 n) { return vec2_add(a, vec2_mulvf(n, -2.0f * vec2_dot(n, a))); }
inline static Vec2 vec2_project_normal(Vec2 a, Vec2 n) { return vec2_mulvf(n, vec2_dot(a, n)); }
inline static Vec2 vec2_mirror(Vec2 a, Vec2 n) { return vec2_sub(a, vec2_mulvf(n, vec2_dot(a, n) * 2.0f)); }
inline static Vec2 vec2_snap(Vec2 a, float size) { return vec2(snapf(a.x, size), snapf(a.y, size)); }
inline static Vec2 vec2_clamp(Vec2 a, Vec2 min, Vec2 max) { return vec2(clampf(a.x, min.x, max.x), clampf(a.y, min.y, max.y)); }
inline static Vec2 vec2_clampf(Vec2 a, float min, float max) { return vec2_mulvf(vec2_norm(a), clampf(vec2_length(a), min, max)); }
inline static Vec2 vec2_reciprocal(Vec2 a) { return vec2(near_eqf(a.x, 0) ? MAX_FLOAT : 1.0 / a.x, near_eqf(a.y, 0) ? MAX_FLOAT : 1.0 / a.y); }
inline static float vec2_cross(Vec2 a, Vec2 b) { return a.x * b.y - a.y * b.x; }
inline static Vec2 vec2_crossvf(Vec2 v, float f) { return vec2(f * v.y, -f * v.x); }
inline static Vec2 vec2_crossfv(float f, Vec2 v) { return vec2(-f * v.y, f * v.x); }
inline static Vec2 vec2_perp_left(Vec2 v) { return vec2(-v.y, v.x); }
inline static Vec2 vec2_perp_right(Vec2 v) { return vec2(v.y, -v.x); }
inline static bool vec2_valid(Vec2 a)
{
    if (!validf(a.x))
        return false;
    if (!validf(a.y))
        return false;
    return true;
}


#endif