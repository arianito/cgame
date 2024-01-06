#include "quat.h"
#include "rot.h"
#include "defs.h"
#include "scalar.h"

Quat quat_mul(Quat a, Quat b)
{

    Quat q;
    q.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    q.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    q.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
    q.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    return q;
}

Quat quat_mulf(Quat a, float b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
    a.w *= b;
    return a;
}

float quat_dot(Quat a, Quat b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float quat_sqr_length(Quat a) { return quat_dot(a, a); }

float quat_length(Quat a)
{
    return sqrf(quat_sqr_length(a));
}

Quat quat_normalize(Quat a)
{
    float b = quat_sqr_length(a);
    if (b <= EPSILON2)
        return quat_identity;
    float si = inv_sqrtf(b);
    a.x *= si;
    a.y *= si;
    a.z *= si;
    a.w *= si;
    return a;
}

Vec3 quat_unrotate(Quat q, Vec3 b)
{
    Vec3 a = {-q.x, -q.y, -q.z};
    Vec3 cross = vec3_mulf(vec3_cross(a, b), 2.0f);
    b = vec3_add(b, vec3_mulf(cross, q.w));
    b = vec3_add(b, vec3_cross(a, cross));
    return b;
}

Vec3 quat_rotate(Quat q, Vec3 b)
{
    Vec3 a = {q.x, q.y, q.z};
    Vec3 cross = vec3_mulf(vec3_cross(a, b), 2.0f);
    b = vec3_add(b, vec3_mulf(cross, -q.w));
    b = vec3_add(b, vec3_cross(a, cross));
    return b;
}

Vec3 quat_forward(Quat q)
{
    return quat_rotate(q, vec3_forward);
}

Vec3 quat_right(Quat q)
{
    return quat_rotate(q, vec3_right);
}

Vec3 quat_up(Quat q)
{
    return quat_rotate(q, vec3_up);
}
