#include "rot.h"

#include "mat4.h"
#include "scalar.h"

bool rot_eq(Rot a, Rot b)
{
    return (bool)(a.pitch == b.pitch && a.yaw == b.yaw && a.roll == b.roll);
}

bool rot_eq0(Rot a)
{
    return (bool)(a.pitch == 0 && a.yaw == 0 && a.roll == 0);
}

bool rot_near_eq(Rot a, Rot b)
{
    return (bool)(near_eqf(a.pitch, b.pitch) &&
                  near_eqf(a.yaw, b.yaw) & near_eqf(a.roll, b.roll));
}

bool rot_near0(Rot a)
{
    return rot_near_eq(a, rot_zero);
}

Rot rot_add(Rot a, Rot b)
{
    a.pitch += b.pitch;
    a.yaw += b.yaw;
    a.roll += b.roll;
    return a;
}

Rot rot_addf(Rot a, float b)
{
    a.pitch += b;
    a.yaw += b;
    a.roll += b;
    return a;
}

Rot rot_sub(Rot u, Rot v)
{
    u.pitch -= v.pitch;
    u.yaw -= v.yaw;
    u.roll -= v.roll;
    return u;
}

Rot rot_subf(Rot a, float b)
{
    a.pitch -= b;
    a.yaw -= b;
    a.roll -= b;
    return a;
}

Rot rot_mulf(Rot a, float b)
{
    a.pitch *= b;
    a.yaw *= b;
    a.roll *= b;
    return a;
}

Rot rot_neg(Rot a)
{
    a.pitch *= -1.0f;
    a.yaw *= -1.0f;
    a.roll *= -1.0f;
    return a;
}

Rot rot_snap(Rot a, float size)
{
    a.pitch = snapf(a.pitch, size);
    a.yaw = snapf(a.yaw, size);
    a.roll = snapf(a.roll, size);
    return a;
}

Rot rot_inv(Rot a)
{
    a.pitch *= -1;
    a.yaw += 180.0f;
    return a;
}

Rot rot_clamp(Rot a)
{
    a.pitch = clamp_axisf(a.pitch);
    a.yaw = clamp_axisf(a.yaw);
    a.roll = clamp_axisf(a.roll);
    return a;
}

bool rot_nan(Rot a)
{
    return (bool)(!validf(a.pitch) || !validf(a.yaw) || !validf(a.roll));
}

Rot rot_norm(Rot a)
{
    a.pitch = norm_axisf(a.pitch);
    a.yaw = norm_axisf(a.yaw);
    a.roll = norm_axisf(a.roll);
    return a;
}

Rot rot_denorm(Rot a)
{
    a.pitch = clamp_axisf(a.pitch);
    a.yaw = clamp_axisf(a.yaw);
    a.roll = clamp_axisf(a.roll);
    return a;
}

float rot_dist(Rot a, Rot b)
{
    return absf(a.pitch - b.pitch) + absf(a.yaw - b.yaw) + absf(a.roll - b.roll);
}

Rot rot_eqv(Rot a)
{
    a.pitch = 180 - a.pitch;
    a.yaw = 180 + a.yaw;
    a.roll = 180 + a.roll;
    return a;
}

Rot rot_lerp(Rot a, Rot b, float dt)
{
    Rot n = rot_norm(rot_sub(b, a));
    a.pitch = a.pitch + n.pitch * dt;
    a.yaw = a.yaw + n.yaw * dt;
    a.roll = a.roll + n.roll * dt;
    return a;
}

Rot rot_lerp01(Rot a, Rot b, float dt)
{
    return rot_lerp(a, b, clamp01f(dt));
}

Rot rot_rlerp(Rot a, Rot b, float dt)
{
    a.pitch = a.pitch + (b.pitch - a.pitch) * dt;
    a.yaw = a.yaw + (b.yaw - a.yaw) * dt;
    a.roll = a.roll + (b.roll - a.roll) * dt;
    return rot_norm(a);
}

Mat4 rot_matrix(Rot a, Vec3 origin)
{
    float cp = cosdf(a.pitch);
    float sp = sindf(a.pitch);
    float cy = cosdf(a.yaw);
    float sy = sindf(a.yaw);
    float cr = cosdf(a.roll);
    float sr = sindf(a.roll);
    return (Mat4){{{cp * cy, cp * sy, sp, 0.0f},
                   {sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, -sr * cp, 0.0f},
                   {-(cr * sp * cy + sr * sy), cy * sr - cr * sp * sy, cr * cp, 0.0f},
                   {origin.x, origin.y, origin.z, 1.0f}}};
}

Vec3 rot_forward(Rot a)
{
    float pitch = moduluf(a.pitch, 360.0f);
    float yaw = moduluf(a.yaw, 360.0f);
    float cp = cosdf(pitch);
    float sp = sindf(pitch);
    float cy = cosdf(yaw);
    float sy = sindf(yaw);
    Vec3 v;
    v.x = cp * cy;
    v.y = cp * sy;
    v.z = sp;
    return v;
}

Quat rot_quat(Rot a)
{
    float pitch = moduluf(a.pitch, 360.0f);
    float yaw = moduluf(a.yaw, 360.0f);
    float roll = moduluf(a.roll, 360.0f);

    float cp = cosdf(pitch * 0.5f);
    float sp = sindf(pitch * 0.5f);
    float cy = cosdf(yaw * 0.5f);
    float sy = sindf(yaw * 0.5f);
    float cr = cosdf(roll * 0.5f);
    float sr = sindf(roll * 0.5f);

    Quat q;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = -cr * cp * sy + sr * sp * cy;
    q.w = cr * cp * cy + sr * sp * sy;

    return q;
}

Vec3 rot_right(Rot a)
{
    Mat4 m = rot_matrix(a, vec3_zero);
    return mat4_axis(m, UNIT_AXIS_Y);
}

Vec3 rot_up(Rot a)
{
    Mat4 m = rot_matrix(a, vec3_zero);
    return mat4_axis(m, UNIT_AXIS_Z);
}

Vec3 rot_rotate(Rot r, Vec3 b)
{
    return mat4_mulv3(rot_matrix(r, vec3_zero), b, 1);
}

Vec3 rot_unrotate(Rot r, Vec3 b)
{
    return mat4_mulv3(mat4_transpose(rot_matrix(r, vec3_zero)), b, 1);
}

Rot rot_look_at(Vec3 a, Vec3 b)
{
    Vec3 forward = vec3_norm(vec3_sub(b, a));
    return mat4_rot(mat4_from_x(forward));
}