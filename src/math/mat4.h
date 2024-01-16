#ifndef cgame_MAT4_H
#define cgame_MAT4_H

#include <stdint.h>
#include <stdbool.h>

#include "rot_ifc.h"
#include "mat4_ifc.h"
#include "vec3.h"
#include "vec4.h"
#include "defs.h"
#include "plane.h"
#include "rot.h"
#include "scalar.h"


Mat4 mat4_mul(Mat4 a, Mat4 b);
Mat4 mat4_add(Mat4 a, Mat4 b);
Mat4 mat4_sub(Mat4 a, Mat4 b);
Mat4 mat4_mulf(Mat4 a, float b);


inline static Mat4 mat4_transpose(Mat4 a)
{
    return (Mat4){{
        {a.m[0][0], a.m[1][0], a.m[2][0], a.m[3][0]},
        {a.m[0][1], a.m[1][1], a.m[2][1], a.m[3][1]},
        {a.m[0][2], a.m[1][2], a.m[2][2], a.m[3][2]},
        {a.m[0][3], a.m[1][3], a.m[2][3], a.m[3][3]},
    }};
}
inline static Mat4 mat4_mul3(Mat4 a, Mat4 b, Mat4 c)
{
    return mat4_mul(mat4_mul(a, b), c);
}

inline static bool mat4_eq(Mat4 a, Mat4 b)
{
    for (uint8_t j = 0; j < 4; j++)
        for (uint8_t i = 0; i < 4; i++)
            if (a.m[j][i] != b.m[j][i])
                return false;
    return true;
}

inline static bool mat4_near_eq(Mat4 a, Mat4 b)
{

    for (uint8_t j = 0; j < 4; j++)
        for (uint8_t i = 0; i < 4; i++)
            if (!near_eqf(a.m[j][i], b.m[j][i]))
                return false;
    return true;
}
Vec4 mat4_mulv4(Mat4 a, Vec4 b);
inline static Vec3 mat4_mulv3(Mat4 a, Vec3 b, float w)
{
    Vec4 v = {b.x, b.y, b.z, w};
    v = mat4_mulv4(a, v);
    b.x = v.x;
    b.y = v.y;
    b.z = v.z;
    return b;
}

inline static Mat4 mat4_scale(Vec3 a)
{
    return (Mat4){{
        {a.x, 0, 0, 0},
        {0, a.y, 0, 0},
        {0, 0, a.z, 0},
        {0, 0, 0, 1},
    }};
}

inline static Mat4 mat4_scalef(float a)
{
    return (Mat4){{
        {a, 0, 0, 0},
        {0, a, 0, 0},
        {0, 0, a, 0},
        {0, 0, 0, 1},
    }};
}

inline static bool mat4_valid(Mat4 a)
{
    for (uint8_t j = 0; j < 4; j++)
        for (uint8_t i = 0; i < 4; i++)
            if (!validf(a.m[j][i]))
                return false;
    return true;
}

inline static Mat4 mat4_origin(Vec3 a)
{
    Mat4 m;
    m.m[0][0] = 1.0f;
    m.m[0][1] = 0.0f;
    m.m[0][2] = 0.0f;
    m.m[0][3] = 0.0f;

    m.m[1][0] = 0.0f;
    m.m[1][1] = 1.0f;
    m.m[1][2] = 0.0f;
    m.m[1][3] = 0.0f;

    m.m[2][0] = 0.0f;
    m.m[2][1] = 0.0f;
    m.m[2][2] = 1.0f;
    m.m[2][3] = 0.0f;

    m.m[3][0] = a.x;
    m.m[3][1] = a.y;
    m.m[3][2] = a.z;
    m.m[3][3] = 1.0f;

    return m;
}

inline static Vec3 mat4_axis(Mat4 a, UnitAxisEnum ax)
{
    Vec3 b;
    switch (ax)
    {
    case UNIT_AXIS_X:
        b.x = a.m[0][0];
        b.y = a.m[0][1];
        b.z = a.m[0][2];
        return b;

    case UNIT_AXIS_Y:
        b.x = a.m[1][0];
        b.y = a.m[1][1];
        b.z = a.m[1][2];
        return b;

    case UNIT_AXIS_Z:
        b.x = a.m[2][0];
        b.y = a.m[2][1];
        b.z = a.m[2][2];
        return b;
    }
    return vec3_zero;
}

inline static void mat4_axes(Mat4 a, Vec3 *ax, Vec3 *ay, Vec3 *az)
{
    ax->x = a.m[0][0];
    ax->y = a.m[0][1];
    ax->z = a.m[0][2];
    ay->x = a.m[1][0];
    ay->y = a.m[1][1];
    ay->z = a.m[1][2];
    az->x = a.m[2][0];
    az->y = a.m[2][1];
    az->z = a.m[2][2];
}

inline static Mat4 mat4_orthographic(float left, float right, float bottom, float top, float nr, float far, float ofc)
{
    Mat4 m;

    float fmd = far - nr;
    float fpd = far + nr;
    float tmb = top - bottom;
    float tpb = top + bottom;
    float rml = right - left;
    float rpl = right + left;

    m.m[0][0] = 2.0f / rml;
    m.m[0][1] = 0.0f;
    m.m[0][2] = 0.0f;
    m.m[0][3] = 0.0f;

    m.m[1][0] = 0.0f;
    m.m[1][1] = 2.0f / tmb;
    m.m[1][2] = 0.0f;
    m.m[1][3] = 0.0f;

    m.m[2][0] = 0.0f;
    m.m[2][1] = ofc;
    m.m[2][2] = 1.0f / fmd;
    m.m[2][3] = 0.0f;

    m.m[3][0] = -rpl / rml;
    m.m[3][1] = -tpb / tmb;
    m.m[3][2] = -fpd / fmd;
    m.m[3][3] = 1.0f;

    return m;
}

inline static Mat4 mat4_perspective(float fov, float aspect, float nr, float fr, float ofc)
{
    float t = tandf(fov * 0.5f);
    float inv = 1.0f / (fr - nr);

    Mat4 m;
    m.m[0][0] = 1.0f / t;
    m.m[0][1] = 0.0f;
    m.m[0][2] = 0.0f;
    m.m[0][3] = 0.0f;

    m.m[1][0] = 0.0f;
    m.m[1][1] = aspect / t;
    m.m[1][2] = 0.0f;
    m.m[1][3] = 0.0f;

    m.m[2][0] = 0.0f;
    m.m[2][1] = ofc;
    m.m[2][2] = (fr + nr) * inv;
    m.m[2][3] = 1.0f;

    m.m[3][0] = 0.0f;
    m.m[3][1] = 0.0f;
    m.m[3][2] = -(2 * nr * fr) * inv;
    m.m[3][3] = 0.0f;

    return m;
}

inline static Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up)
{
    Mat4 m;
    Vec3 zaxis = vec3_norm(vec3_sub(center, eye));
    Vec3 xaxis = vec3_norm(vec3_cross(up, zaxis));
    Vec3 yaxis = vec3_cross(zaxis, xaxis);

    m.m[0][0] = xaxis.x;
    m.m[1][0] = xaxis.y;
    m.m[2][0] = xaxis.z;
    m.m[3][0] = -vec3_dot(eye, xaxis);
    m.m[0][1] = yaxis.x;
    m.m[1][1] = yaxis.y;
    m.m[2][1] = yaxis.z;
    m.m[3][1] = -vec3_dot(eye, yaxis);
    m.m[0][2] = zaxis.x;
    m.m[1][2] = zaxis.y;
    m.m[2][2] = zaxis.z;
    m.m[3][2] = -vec3_dot(eye, zaxis);
    m.m[0][3] = 0.0f;
    m.m[1][3] = 0.0f;
    m.m[2][3] = 0.0f;
    m.m[3][3] = 1.0f;

    return m;
}

inline static Mat4 mat4_vec3(Vec3 xa, Vec3 ya, Vec3 za, Vec3 wa)
{
    Mat4 m;
    m.m[0][0] = xa.x;
    m.m[0][1] = xa.y;
    m.m[0][2] = xa.z;
    m.m[0][3] = 0.0f;

    m.m[1][0] = ya.x;
    m.m[1][1] = ya.y;
    m.m[1][2] = ya.z;
    m.m[1][3] = 0.0f;

    m.m[2][0] = za.x;
    m.m[2][1] = za.y;
    m.m[2][2] = za.z;
    m.m[2][3] = 0.0f;

    m.m[3][0] = wa.x;
    m.m[3][1] = wa.y;
    m.m[3][2] = wa.z;
    m.m[3][3] = 1.0f;
    return m;
}

inline static Rot mat4_rot(Mat4 m)
{
    Vec3 ax, ay, az;
    mat4_axes(m, &ax, &ay, &az);

    Rot r = rot(
        atan2df(ax.z, sqrf(squaref(ax.x) + squaref(ax.y))),
        atan2df(ax.y, ax.x),
        0);

    Mat4 m2 = rot_matrix(r, vec3_zero);
    Vec3 say = mat4_axis(m2, UNIT_AXIS_Y);
    r.roll = atan2df(vec3_dot(az, say), vec3_dot(ay, say));
    return r;
}

inline static Mat4 mat4_inverse_rot(Rot a)
{
    float cy = cosdf(a.yaw);
    float sy = sindf(a.yaw);
    float cp = cosdf(a.pitch);
    float sp = sindf(a.pitch);
    float cr = cosdf(a.roll);
    float sr = sindf(a.roll);
    Mat4 ma = {{
        // z
        {+cy, -sy, 0.f, 0.f},
        {+sy, +cy, 0.f, 0.f},
        {0.f, 0.f, 1.f, 0.f},
        {0.f, 0.f, 0.f, 1.f},
    }};
    Mat4 mb = {{
        // y
        {+cp, 0.f, -sp, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {+sp, 0.f, +cp, 0.f},
        {0.f, 0.f, 0.f, 1.f},
    }};
    Mat4 mc = {{
        // x
        {1.f, 0.f, 0.f, 0.f},
        {0.f, +cr, -sr, 0.f},
        {0.f, +sr, +cr, 0.f},
        {0.f, 0.f, 0.f, 1.f},
    }};
    Mat4 m = mat4_mul3(ma, mb, mc);

    return m;
}

inline static Mat4 mat4_from_x(Vec3 x)
{
    x = vec3_norm(x);
    Vec3 up = (absf(x.z) < (1.f - EPSILON)) ? vec3(0, 0, 1.0f) : vec3(1.0f, 0, 0);
    Vec3 y = vec3_norm(vec3_cross(up, x));
    Vec3 z = vec3_cross(x, y);
    return mat4_vec3(x, y, z, vec3_zero);
}

inline static Mat4 mat4_from_y(Vec3 y)
{
    y = vec3_norm(y);
    Vec3 up = (absf(y.z) < (1.f - EPSILON)) ? vec3(0, 0, 1.0f) : vec3(1.0f, 0, 0);
    Vec3 z = vec3_norm(vec3_cross(up, y));
    Vec3 x = vec3_cross(y, z);
    return mat4_vec3(x, y, z, vec3_zero);
}

inline static Mat4 mat4_from_z(Vec3 z)
{
    z = vec3_norm(z);
    Vec3 up = (absf(z.z) < (1.f - EPSILON)) ? vec3(0, 0, 1.0f) : vec3(1.0f, 0, 0);
    Vec3 x = vec3_norm(vec3_cross(up, z));
    Vec3 y = vec3_cross(z, x);
    return mat4_vec3(x, y, z, vec3_zero);
}

inline static Mat4 mat4_view(Vec3 a, Rot b)
{
    Mat4 ma = {{{0, 0, 1, 0},
                {1, 0, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 0, 1}}};
    return mat4_mul(
        mat4_origin(vec3_neg(a)),
        mat4_mul(mat4_inverse_rot(b), ma));
}

inline static Mat4 mat4_transform(Vec3 origin, float scale)
{
    Mat4 mt = {{{scale, 0, 0, 0},
                {0, scale, 0, 0},
                {0, 0, scale, 0},
                {origin.x, origin.y, origin.z, 1}}};
    return mt;
}

inline static float mat4_det(Mat4 m)
{
    return m.m[0][0] * (m.m[1][1] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) -
                        m.m[2][1] * (m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2]) +
                        m.m[3][1] * (m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2])) -
           m.m[1][0] * (m.m[0][1] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) -
                        m.m[2][1] * (m.m[0][2] * m.m[3][3] - m.m[0][3] * m.m[3][2]) +
                        m.m[3][1] * (m.m[0][2] * m.m[2][3] - m.m[0][3] * m.m[2][2])) +
           m.m[2][0] * (m.m[0][1] * (m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2]) -
                        m.m[1][1] * (m.m[0][2] * m.m[3][3] - m.m[0][3] * m.m[3][2]) +
                        m.m[3][1] * (m.m[0][2] * m.m[1][3] - m.m[0][3] * m.m[1][2])) -
           m.m[3][0] * (m.m[0][1] * (m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2]) -
                        m.m[1][1] * (m.m[0][2] * m.m[2][3] - m.m[0][3] * m.m[2][2]) +
                        m.m[2][1] * (m.m[0][2] * m.m[1][3] - m.m[0][3] * m.m[1][2]));
}

inline static Mat4 mat4_inv(Mat4 m)
{

    float da[4];
    Mat4 tmp, res;

    tmp.m[0][0] = m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2];
    tmp.m[0][1] = m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2];
    tmp.m[0][2] = m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2];

    tmp.m[1][0] = m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2];
    tmp.m[1][1] = m.m[0][2] * m.m[3][3] - m.m[0][3] * m.m[3][2];
    tmp.m[1][2] = m.m[0][2] * m.m[2][3] - m.m[0][3] * m.m[2][2];

    tmp.m[2][0] = m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2];
    tmp.m[2][1] = m.m[0][2] * m.m[3][3] - m.m[0][3] * m.m[3][2];
    tmp.m[2][2] = m.m[0][2] * m.m[1][3] - m.m[0][3] * m.m[1][2];

    tmp.m[3][0] = m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2];
    tmp.m[3][1] = m.m[0][2] * m.m[2][3] - m.m[0][3] * m.m[2][2];
    tmp.m[3][2] = m.m[0][2] * m.m[1][3] - m.m[0][3] * m.m[1][2];

    da[0] = m.m[1][1] * tmp.m[0][0] - m.m[2][1] * tmp.m[0][1] +
            m.m[3][1] * tmp.m[0][2];
    da[1] = m.m[0][1] * tmp.m[1][0] - m.m[2][1] * tmp.m[1][1] +
            m.m[3][1] * tmp.m[1][2];
    da[2] = m.m[0][1] * tmp.m[2][0] - m.m[1][1] * tmp.m[2][1] +
            m.m[3][1] * tmp.m[2][2];
    da[3] = m.m[0][1] * tmp.m[3][0] - m.m[1][1] * tmp.m[3][1] +
            m.m[2][1] * tmp.m[3][2];

    const float det =
        m.m[0][0] * da[0] - m.m[1][0] * da[1] + m.m[2][0] * da[2] -
        m.m[3][0] * da[3];
    const float rdet = 1.0f / det;

    res.m[0][0] = rdet * da[0];
    res.m[0][1] = -rdet * da[1];
    res.m[0][2] = rdet * da[2];
    res.m[0][3] = -rdet * da[3];
    res.m[1][0] = -rdet * (m.m[1][0] * tmp.m[0][0] - m.m[2][0] * tmp.m[0][1] +
                           m.m[3][0] * tmp.m[0][2]);
    res.m[1][1] = rdet * (m.m[0][0] * tmp.m[1][0] - m.m[2][0] * tmp.m[1][1] +
                          m.m[3][0] * tmp.m[1][2]);
    res.m[1][2] = -rdet * (m.m[0][0] * tmp.m[2][0] - m.m[1][0] * tmp.m[2][1] +
                           m.m[3][0] * tmp.m[2][2]);
    res.m[1][3] = rdet * (m.m[0][0] * tmp.m[3][0] - m.m[1][0] * tmp.m[3][1] +
                          m.m[2][0] * tmp.m[3][2]);
    res.m[2][0] = rdet *
                  (m.m[1][0] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) -
                   m.m[2][0] * (m.m[1][1] * m.m[3][3] - m.m[1][3] * m.m[3][1]) +
                   m.m[3][0] * (m.m[1][1] * m.m[2][3] - m.m[1][3] * m.m[2][1]));
    res.m[2][1] = -rdet *
                  (m.m[0][0] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) -
                   m.m[2][0] * (m.m[0][1] * m.m[3][3] - m.m[0][3] * m.m[3][1]) +
                   m.m[3][0] * (m.m[0][1] * m.m[2][3] - m.m[0][3] * m.m[2][1]));
    res.m[2][2] = rdet *
                  (m.m[0][0] * (m.m[1][1] * m.m[3][3] - m.m[1][3] * m.m[3][1]) -
                   m.m[1][0] * (m.m[0][1] * m.m[3][3] - m.m[0][3] * m.m[3][1]) +
                   m.m[3][0] * (m.m[0][1] * m.m[1][3] - m.m[0][3] * m.m[1][1]));
    res.m[2][3] = -rdet *
                  (m.m[0][0] * (m.m[1][1] * m.m[2][3] - m.m[1][3] * m.m[2][1]) -
                   m.m[1][0] * (m.m[0][1] * m.m[2][3] - m.m[0][3] * m.m[2][1]) +
                   m.m[2][0] * (m.m[0][1] * m.m[1][3] - m.m[0][3] * m.m[1][1]));
    res.m[3][0] = -rdet *
                  (m.m[1][0] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1]) -
                   m.m[2][0] * (m.m[1][1] * m.m[3][2] - m.m[1][2] * m.m[3][1]) +
                   m.m[3][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]));
    res.m[3][1] = rdet *
                  (m.m[0][0] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1]) -
                   m.m[2][0] * (m.m[0][1] * m.m[3][2] - m.m[0][2] * m.m[3][1]) +
                   m.m[3][0] * (m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1]));
    res.m[3][2] = -rdet *
                  (m.m[0][0] * (m.m[1][1] * m.m[3][2] - m.m[1][2] * m.m[3][1]) -
                   m.m[1][0] * (m.m[0][1] * m.m[3][2] - m.m[0][2] * m.m[3][1]) +
                   m.m[3][0] * (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]));
    res.m[3][3] = rdet *
                  (m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) -
                   m.m[1][0] * (m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1]) +
                   m.m[2][0] * (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]));

    return res;
}

#endif