#ifndef cgame_VEC3_H
#define cgame_VEC3_H

#include <stdbool.h>
#include <stdint.h>

#include "scalar.h"
#include "defs.h"
#include "rand.h"
#include "vec3_ifc.h"
#include "vec2_ifc.h"

inline static Vec3 vec3yz(Vec2 a) { return vec3(0, a.x, a.y); }
inline static Vec3 vec3xy(Vec2 a) { return vec3(a.x, a.y, 0); }
inline static Vec3 vec3xz(Vec2 a) { return vec3(a.x, 0, a.y); }

inline static Vec3 vec3_rotate_xy(Vec3 a, float d)
{
    return vec3(
        a.x * sindf(d) + a.y * cosdf(d),
        a.x * cosdf(d) - a.y * sindf(d),
        a.z);
}

inline static Vec3 vec3_rotate_yz(Vec3 a, float d)
{
    return vec3(
        a.x,
        a.y * sindf(d) + a.z * cosdf(d),
        a.y * cosdf(d) - a.z * sindf(d));
}

inline static Vec3 vec3_randv(Vec3 halfBound)
{
    return vec3(
        randf() * halfBound.x * 2.0f - halfBound.x,
        randf() * halfBound.y * 2.0f - halfBound.y,
        randf() * halfBound.z * 2.0f - halfBound.z);
}

inline static Vec3 vec3_randf(float x, float y, float z)
{
    return vec3(
        randf() * x * 2.0f - x,
        randf() * y * 2.0f - y,
        randf() * z * 2.0f - z);
}

inline static Vec3 vec3_neg(Vec3 a)
{
    a.x *= -1.0f;
    a.y *= -1.0f;
    a.z *= -1.0f;
    return a;
}

inline static Vec3 vec3_add(Vec3 a, Vec3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}
inline static Vec3 vec3_center(Vec3 a, Vec3 b)
{
    a.x = (a.x + b.x) * 0.5f;
    a.y = (a.y + b.y) * 0.5f;
    a.z = (a.z + b.z) * 0.5f;
    return a;
}

inline static Vec3 vec3_addf(Vec3 a, float b)
{
    a.x += b;
    a.y += b;
    a.z += b;
    return a;
}

inline static Vec3 vec3_sub(Vec3 u, Vec3 v)
{
    u.x -= v.x;
    u.y -= v.y;
    u.z -= v.z;
    return u;
}

inline static Vec3 vec3_subf(Vec3 a, float b)
{
    a.x -= b;
    a.y -= b;
    a.z -= b;
    return a;
}

inline static Vec3 vec3_mulf(Vec3 a, float b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
    return a;
}

inline static float vec3_dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline static float vec3_sqr_length(Vec3 a)
{
    return vec3_dot(a, a);
}

inline static float vec3_length(Vec3 a)
{
    return sqrf(vec3_dot(a, a));
}

inline static float vec3_dist(Vec3 a, Vec3 b)
{
    return vec3_length(vec3_sub(a, b));
}

inline static float vec3_dot_xy(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y;
}

inline static float vec3_sqr_length_xy(Vec3 a)
{
    return vec3_dot_xy(a, a);
}

inline static float vec3_length_xy(Vec3 a)
{
    return sqrf(vec3_dot_xy(a, a));
}

inline static float vec3_distance_xy(Vec3 a, Vec3 b)
{

    return vec3_length_xy(vec3_sub(a, b));
}

inline static bool vec3_eq(Vec3 a, Vec3 b)
{
    return (bool)(a.x == b.x && a.y == b.y && a.z == b.z);
}

inline static bool vec3_eq0(Vec3 a)
{
    return (bool)(a.x == 0 && a.y == 0 && a.z == 0);
}

inline static bool vec3_near_eq(Vec3 a, Vec3 b)
{
    return (bool)(vec3_dist(a, b) < EPSILON);
}

inline static bool vec3_near0(Vec3 a)
{
    return vec3_near_eq(a, vec3_zero);
}

inline static Vec3 vec3_cross(Vec3 a, Vec3 b)
{
    Vec3 c;
    c.x = a.y * b.z - a.z * b.y;
    c.y = a.z * b.x - a.x * b.z;
    c.z = a.x * b.y - a.y * b.x;
    return c;
}

inline static Vec3 vec3_min(Vec3 a, Vec3 b)
{
    a.x = minf(a.x, b.x);
    a.y = minf(a.y, b.y);
    a.z = minf(a.z, b.z);
    return a;
}

inline static float vec3_min_comp(Vec3 a)
{
    return minf(minf(a.x, a.y), a.z);
}

inline static Vec3 vec3_max(Vec3 a, Vec3 b)
{
    a.x = maxf(a.x, b.x);
    a.y = maxf(a.y, b.y);
    a.z = maxf(a.z, b.z);
    return a;
}

inline static float vec3_max_comp(Vec3 a)
{
    return maxf(maxf(a.x, a.y), a.z);
}

inline static Vec3 vec3_abs(Vec3 a)
{
    a.x = absf(a.x);
    a.y = absf(a.y);
    a.z = absf(a.z);
    return a;
}

inline static float vec3_abs_min(Vec3 a)
{
    return minf(minf(absf(a.x), absf(a.y)), absf(a.z));
}

inline static float vec3_abs_max(Vec3 a)
{
    return maxf(maxf(absf(a.x), absf(a.y)), absf(a.z));
}

inline static Vec3 vec3_norm(Vec3 a)
{
    float s = vec3_sqr_length(a);
    if (s == 1.0f)
        return a;
    if (s <= EPSILON2)
        return vec3_zero;
    float si = inv_sqrtf(s);
    a.x *= si;
    a.y *= si;
    a.z *= si;
    return a;
}

inline static Vec3 vec3_norm_xy(Vec3 a)
{
    float s = vec3_sqr_length_xy(a);
    if (s == 1)
    {
        if (a.z == 0)
            return a;
        a.z = 0;
        return a;
    }
    if (s <= EPSILON2)
        return vec3_zero;

    float si = inv_sqrtf(s);
    a.x *= si;
    a.y *= si;
    a.z = 0;
    return a;
}

inline static Vec3 vec3_sign(Vec3 a)
{
    a.x = signf(a.x);
    a.y = signf(a.y);
    a.z = signf(a.z);
    return a;
}

inline static Vec3 vec3_lerp(Vec3 a, Vec3 b, float dt)
{
    a.x = a.x + (b.x - a.x) * dt;
    a.y = a.y + (b.y - a.y) * dt;
    a.z = a.z + (b.z - a.z) * dt;
    return a;
}

inline static Vec3 vec3_lerp01(Vec3 a, Vec3 b, float dt)
{
    return vec3_lerp(a, b, clamp01f(dt));
}

inline static Vec3 vec3_move_towards(Vec3 current, Vec3 target, float maxDelta)
{
    Vec3 toVector = vec3_sub(target, current);
    float dist = vec3_length(toVector);
    if (dist <= maxDelta || dist < EPSILON)
        return target;
    return vec3_add(current, vec3_mulf(toVector, maxDelta / dist));
}

inline static Vec3 vec3_reflect(Vec3 a, Vec3 n)
{
    return vec3_add(a, vec3_mulf(n, -2.0f * vec3_dot(n, a)));
}

inline static Vec3 vec3_projection(Vec3 a)
{
    float rz = 1.0f / a.z;
    a.x = a.x * rz;
    a.y = a.y * rz;
    a.z = 0;
    return a;
}

inline static Vec3 vec3_project(Vec3 a, Vec3 b)
{
    float s = vec3_sqr_length(b);
    if (s <= EPSILON2)
        return vec3_zero;
    return vec3_mulf(b, vec3_dot(a, b) * inv_sqrtf(s));
}

inline static Vec3 vec3_project_normal(Vec3 a, Vec3 n)
{
    return vec3_mulf(n, vec3_dot(a, n));
}

inline static Vec3 vec3_mirror(Vec3 a, Vec3 n)
{
    return vec3_sub(a, vec3_mulf(n, vec3_dot(a, n) * 2.0f));
}

inline static Vec3 vec3_snap(Vec3 a, float size)
{
    a.x = snapf(a.x, size);
    a.y = snapf(a.y, size);
    a.z = snapf(a.z, size);
    return a;
}

inline static Vec3 vec3_snap_cube(Vec3 a, float size)
{
    float hs = size / 2;
    a.x = snapf(-hs + a.x, size) + hs;
    a.y = snapf(-hs + a.y, size) + hs;
    a.z = snapf(-hs + a.z, size) + hs;
    return a;
}

inline static Vec3 vec3_snap_cube_xy(Vec3 a, float size)
{
    float hs = size / 2;
    a.x = snapf(-hs + a.x, size) + hs;
    a.y = snapf(-hs + a.y, size) + hs;
    a.z = 0;
    return a;
}
inline static Vec3 vec3_clamp(Vec3 a, Vec3 min, Vec3 max)
{
    a.x = clampf(a.x, min.x, max.x);
    a.y = clampf(a.y, min.y, max.y);
    a.z = clampf(a.z, min.z, max.z);
    return a;
}

inline static Vec3 vec3_clampf(Vec3 a, float min, float max)
{
    float sz = clampf(vec3_length(a), min, max);
    return vec3_mulf(vec3_norm(a), sz);
}

inline static Vec3 vec3_clampf_xy(Vec3 a, float min, float max)
{
    float z = a.z;
    a = vec3_mulf(vec3_norm(a), clampf(vec3_length_xy(a), min, max));
    a.z = z;
    return a;
}

inline static Vec3 vec3_reciprocal(Vec3 a)
{
    if (near_eqf(a.x, 0))
        a.x = MAX_FLOAT;
    else
        a.x = 1.0f / a.x;

    if (near_eqf(a.y, 0))
        a.y = MAX_FLOAT;
    else
        a.y = 1.0f / a.y;

    if (near_eqf(a.z, 0))
        a.z = MAX_FLOAT;
    else
        a.z = 1.0f / a.z;

    return a;
}

inline static float vec3_angle(Vec3 a, Vec3 b)
{
    float de = vec3_sqr_length(a) * vec3_sqr_length(b);
    if (de <= EPSILON2)
        return 0.0f;
    return acosdf(clampf(vec3_dot(a, b) * inv_sqrtf(de), -1.0f, 1.0f));
}

inline static float vec3_signed_angle(Vec3 a, Vec3 b, Vec3 axis)
{
    return vec3_angle(a, b) * signf(vec3_dot(axis, vec3_cross(a, b)));
}

inline static float vec3_cos_angle_xy(Vec3 a, Vec3 b)
{
    a.z = 0;
    b.z = 0;
    a = vec3_norm(a);
    b = vec3_norm(b);
    return vec3_dot(a, b);
}

inline static Vec3 vec3_intersect_plane(Vec3 start, Vec3 end, Vec3 planeOrigin, Vec3 planeNormal)
{
    end = vec3_sub(end, start);
    return vec3_add(start, vec3_mulf(end, vec3_dot(vec3_sub(planeOrigin, start), planeNormal) / vec3_dot(end, planeNormal)));
}

#endif