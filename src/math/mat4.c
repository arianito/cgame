#include "mat4.h"

#include <immintrin.h>
#include "rot.h"
#include "scalar.h"

Mat4 mat4_transpose(Mat4 a)
{
    Mat4 m;
    m.m[0][0] = a.m[0][0];
    m.m[0][1] = a.m[1][0];
    m.m[0][2] = a.m[2][0];
    m.m[0][3] = a.m[3][0];
    m.m[1][0] = a.m[0][1];
    m.m[1][1] = a.m[1][1];
    m.m[1][2] = a.m[2][1];
    m.m[1][3] = a.m[3][1];
    m.m[2][0] = a.m[0][2];
    m.m[2][1] = a.m[1][2];
    m.m[2][2] = a.m[2][2];
    m.m[2][3] = a.m[3][2];
    m.m[3][0] = a.m[0][3];
    m.m[3][1] = a.m[1][3];
    m.m[3][2] = a.m[2][3];
    m.m[3][3] = a.m[3][3];
    return m;
}

Mat4 mat4_mul(Mat4 a, Mat4 b)
{

    Mat4 m;
#if defined(USE_AVX_256) || defined(USE_AVX_128)
    __m128 tmp, sum;
    __m128 b0 = _mm_loadu_ps(b.m[0]);
    __m128 b1 = _mm_loadu_ps(b.m[1]);
    __m128 b2 = _mm_loadu_ps(b.m[2]);
    __m128 b3 = _mm_loadu_ps(b.m[3]);

    tmp = _mm_set_ps1(a.m[0][0]);
    sum = _mm_mul_ps(tmp, b0);
    tmp = _mm_set_ps1(a.m[0][1]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b1), sum);
    tmp = _mm_set_ps1(a.m[0][2]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b2), sum);
    tmp = _mm_set_ps1(a.m[0][3]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b3), sum);
    _mm_storeu_ps(m.m[0], sum);

    tmp = _mm_set_ps1(a.m[1][0]);
    sum = _mm_mul_ps(tmp, b0);
    tmp = _mm_set_ps1(a.m[1][1]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b1), sum);
    tmp = _mm_set_ps1(a.m[1][2]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b2), sum);
    tmp = _mm_set_ps1(a.m[1][3]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b3), sum);
    _mm_storeu_ps(m.m[1], sum);

    tmp = _mm_set_ps1(a.m[2][0]);
    sum = _mm_mul_ps(tmp, b0);
    tmp = _mm_set_ps1(a.m[2][1]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b1), sum);
    tmp = _mm_set_ps1(a.m[2][2]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b2), sum);
    tmp = _mm_set_ps1(a.m[2][3]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b3), sum);
    _mm_storeu_ps(m.m[2], sum);

    tmp = _mm_set_ps1(a.m[3][0]);
    sum = _mm_mul_ps(tmp, b0);
    tmp = _mm_set_ps1(a.m[3][1]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b1), sum);
    tmp = _mm_set_ps1(a.m[3][2]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b2), sum);
    tmp = _mm_set_ps1(a.m[3][3]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b3), sum);
    _mm_storeu_ps(m.m[3], sum);
#else
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            m.m[i][j] = 0;
            for (int k = 0; k < 4; ++k)
            {
                m.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
#endif
    return m;
}

Mat4 mat4_mul3(Mat4 a, Mat4 b, Mat4 c)
{
    return mat4_mul(mat4_mul(a, b), c);
}

Mat4 mat4_add(Mat4 a, Mat4 b)
{
#if defined(USE_AVX_256)
    __m256 a0, b0;
    a0 = _mm256_loadu_ps(a.m[0]);
    b0 = _mm256_loadu_ps(b.m[0]);
    a0 = _mm256_add_ps(a0, b0);
    _mm256_storeu_ps(a.m[0], a0);

    a0 = _mm256_loadu_ps(a.m[2]);
    b0 = _mm256_loadu_ps(b.m[2]);
    a0 = _mm256_add_ps(a0, b0);
    _mm256_storeu_ps(a.m[2], a0);
#elif defined(USE_AVX_128)
    __m128 a0, b0;
    a0 = _mm_loadu_ps(a.m[0]);
    b0 = _mm_loadu_ps(b.m[0]);
    a0 = _mm_add_ps(a0, b0);
    _mm_storeu_ps(a.m[0], a0);
    a0 = _mm_loadu_ps(a.m[1]);
    b0 = _mm_loadu_ps(b.m[1]);
    a0 = _mm_add_ps(a0, b0);
    _mm_storeu_ps(a.m[1], a0);
    a0 = _mm_loadu_ps(a.m[2]);
    b0 = _mm_loadu_ps(b.m[2]);
    a0 = _mm_add_ps(a0, b0);
    _mm_storeu_ps(a.m[2], a0);
    a0 = _mm_loadu_ps(a.m[3]);
    b0 = _mm_loadu_ps(b.m[3]);
    a0 = _mm_add_ps(a0, b0);
    _mm_storeu_ps(a.m[3], a0);
#else
    for (int j = 0; j < 4; j++)
        for (int i = 0; i < 4; i++)
            a.m[j][i] += b.m[j][i];
#endif
    return a;
}

Mat4 mat4_sub(Mat4 a, Mat4 b)
{
#if defined(USE_AVX_256)
    __m256 a0, b0;
    a0 = _mm256_loadu_ps(a.m[0]);
    b0 = _mm256_loadu_ps(b.m[0]);
    a0 = _mm256_sub_ps(a0, b0);
    _mm256_storeu_ps(a.m[0], a0);
    a0 = _mm256_loadu_ps(a.m[2]);
    b0 = _mm256_loadu_ps(b.m[2]);
    a0 = _mm256_sub_ps(a0, b0);
    _mm256_storeu_ps(a.m[2], a0);
#elif defined(USE_AVX_128)
    __m128 a0, b0;
    a0 = _mm_loadu_ps(a.m[0]);
    b0 = _mm_loadu_ps(b.m[0]);
    a0 = _mm_sub_ps(a0, b0);
    _mm_storeu_ps(a.m[0], a0);
    a0 = _mm_loadu_ps(a.m[1]);
    b0 = _mm_loadu_ps(b.m[1]);
    a0 = _mm_sub_ps(a0, b0);
    _mm_storeu_ps(a.m[1], a0);
    a0 = _mm_loadu_ps(a.m[2]);
    b0 = _mm_loadu_ps(b.m[2]);
    a0 = _mm_sub_ps(a0, b0);
    _mm_storeu_ps(a.m[2], a0);
    a0 = _mm_loadu_ps(a.m[3]);
    b0 = _mm_loadu_ps(b.m[3]);
    a0 = _mm_sub_ps(a0, b0);
    _mm_storeu_ps(a.m[3], a0);
#else
    for (int j = 0; j < 4; j++)
        for (int i = 0; i < 4; i++)
            a.m[j][i] -= b.m[j][i];
#endif
    return a;
}

Mat4 mat4_mulf(Mat4 a, float b)
{
#if defined(USE_AVX_256)
    __m256 a0, b0;
    b0 = _mm256_set1_ps(b);
    a0 = _mm256_loadu_ps(a.m[0]);
    a0 = _mm256_mul_ps(a0, b0);
    _mm256_storeu_ps(a.m[0], a0);
    a0 = _mm256_loadu_ps(a.m[2]);
    a0 = _mm256_mul_ps(a0, b0);
    _mm256_storeu_ps(a.m[2], a0);
#elif defined(USE_AVX_128)
    __m128 a0, b0;
    b0 = _mm_set1_ps(b);
    a0 = _mm_loadu_ps(a.m[0]);
    a0 = _mm_mul_ps(a0, b0);
    _mm_storeu_ps(a.m[0], a0);
    a0 = _mm_loadu_ps(a.m[1]);
    a0 = _mm_mul_ps(a0, b0);
    _mm_storeu_ps(a.m[1], a0);
    a0 = _mm_loadu_ps(a.m[2]);
    a0 = _mm_mul_ps(a0, b0);
    _mm_storeu_ps(a.m[2], a0);
    a0 = _mm_loadu_ps(a.m[3]);
    a0 = _mm_mul_ps(a0, b0);
    _mm_storeu_ps(a.m[3], a0);
#else
    for (int j = 0; j < 4; j++)
        for (int i = 0; i < 4; i++)
            a.m[j][i] *= b;
#endif
    return a;
}

bool mat4_eq(Mat4 a, Mat4 b)
{
    for (uint8_t j = 0; j < 4; j++)
        for (uint8_t i = 0; i < 4; i++)
            if (a.m[j][i] != b.m[j][i])
                return false;
    return true;
}

bool mat4_near_eq(Mat4 a, Mat4 b)
{

    for (uint8_t j = 0; j < 4; j++)
        for (uint8_t i = 0; i < 4; i++)
            if (!near_eqf(a.m[j][i], b.m[j][i]))
                return false;
    return true;
}

Vec4 mat4_mulv4(Mat4 a, Vec4 b)
{
#if defined(USE_AVX_256) || defined(USE_AVX_128)

    __m128 tmp, sum;
    __m128 b0 = _mm_loadu_ps(a.m[0]);
    __m128 b1 = _mm_loadu_ps(a.m[1]);
    __m128 b2 = _mm_loadu_ps(a.m[2]);
    __m128 b3 = _mm_loadu_ps(a.m[3]);

    tmp = _mm_set_ps1(b.x);
    sum = _mm_mul_ps(tmp, b0);
    tmp = _mm_set_ps1(b.y);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b1), sum);
    tmp = _mm_set_ps1(b.z);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b2), sum);
    tmp = _mm_set_ps1(b.w);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b3), sum);
    _mm_storeu_ps((float *)(&b), sum);
    return b;

#else
#endif
    return b;
}

Vec3 mat4_mulv3(Mat4 a, Vec3 b, float w)
{
    Vec4 v = {b.x, b.y, b.z, w};
    v = mat4_mulv4(a, v);
    b.x = v.x;
    b.y = v.y;
    b.z = v.z;
    return b;
}

Mat4 mat4_scale(Vec3 a)
{
    Mat4 m;

    m.m[0][0] = a.x;
    m.m[0][1] = 0.0f;
    m.m[0][2] = 0.0f;
    m.m[0][3] = 0.0f;

    m.m[1][0] = 0.0f;
    m.m[1][1] = a.y;
    m.m[1][2] = 0.0f;
    m.m[1][3] = 0.0f;

    m.m[2][0] = 0.0f;
    m.m[2][1] = 0.0f;
    m.m[2][2] = a.z;
    m.m[2][3] = 0.0f;

    m.m[3][0] = 0.0f;
    m.m[3][1] = 0.0f;
    m.m[3][2] = 0.0f;
    m.m[3][3] = 1.0f;

    return m;
}

Mat4 mat4_scalef(float a)
{

    Mat4 m;

    m.m[0][0] = a;
    m.m[0][1] = 0.0f;
    m.m[0][2] = 0.0f;
    m.m[0][3] = 0.0f;

    m.m[1][0] = 0.0f;
    m.m[1][1] = a;
    m.m[1][2] = 0.0f;
    m.m[1][3] = 0.0f;

    m.m[2][0] = 0.0f;
    m.m[2][1] = 0.0f;
    m.m[2][2] = a;
    m.m[2][3] = 0.0f;

    m.m[3][0] = 0.0f;
    m.m[3][1] = 0.0f;
    m.m[3][2] = 0.0f;
    m.m[3][3] = 1.0f;

    return m;
}

bool mat4_valid(Mat4 a)
{
    for (uint8_t j = 0; j < 4; j++)
        for (uint8_t i = 0; i < 4; i++)
            if (!validf(a.m[j][i]))
                return false;
    return true;
}

Mat4 mat4_origin(Vec3 a)
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

Vec3 mat4_axis(Mat4 a, UnitAxisEnum ax)
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

void mat4_axes(Mat4 a, Vec3 *ax, Vec3 *ay, Vec3 *az)
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

Mat4 mat4_orthographic(float left, float right, float bottom, float top, float nr, float far)
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
    m.m[2][1] = 0.0f;
    m.m[2][2] = 1.0f / fmd;
    m.m[2][3] = 0.0f;

    m.m[3][0] = -rpl / rml;
    m.m[3][1] = -tpb / tmb;
    m.m[3][2] = -fpd / fmd;
    m.m[3][3] = 1.0f;

    return m;
}

Mat4 mat4_perspective(float fov, float aspect, float nr, float fr)
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
    m.m[2][1] = 0.0f;
    m.m[2][2] = (fr + nr) * inv;
    m.m[2][3] = 1.0f;

    m.m[3][0] = 0.0f;
    m.m[3][1] = 0.0f;
    m.m[3][2] = -(2 * nr * fr) * inv;
    m.m[3][3] = 0.0f;

    return m;
}

Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up)
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

Mat4 mat4_vec3(Vec3 xa, Vec3 ya, Vec3 za, Vec3 wa)
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

Mat4 mat4_plane(Plane a, Plane b, Plane c, Plane d)
{
    Mat4 m;
    m.m[0][0] = a.x;
    m.m[0][1] = a.y;
    m.m[0][2] = a.z;
    m.m[0][3] = a.w;
    m.m[1][0] = b.x;
    m.m[1][1] = b.y;
    m.m[1][2] = b.z;
    m.m[1][3] = b.w;
    m.m[2][0] = c.x;
    m.m[2][1] = c.y;
    m.m[2][2] = c.z;
    m.m[2][3] = c.w;
    m.m[3][0] = d.x;
    m.m[3][1] = d.y;
    m.m[3][2] = d.z;
    m.m[3][3] = d.w;
    return m;
}

Rot mat4_rot(Mat4 m)
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

Mat4 mat4_inverse_rot(Rot a)
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

Mat4 mat4_from_x(Vec3 x)
{
    x = vec3_norm(x);
    Vec3 up = (absf(x.z) < (1.f - EPSILON)) ? vec3(0, 0, 1.0f) : vec3(1.0f, 0, 0);
    Vec3 y = vec3_norm(vec3_cross(up, x));
    Vec3 z = vec3_cross(x, y);
    return mat4_vec3(x, y, z, vec3_zero);
}

Mat4 mat4_from_y(Vec3 y)
{
    y = vec3_norm(y);
    Vec3 up = (absf(y.z) < (1.f - EPSILON)) ? vec3(0, 0, 1.0f) : vec3(1.0f, 0, 0);
    Vec3 z = vec3_norm(vec3_cross(up, y));
    Vec3 x = vec3_cross(y, z);
    return mat4_vec3(x, y, z, vec3_zero);
}

Mat4 mat4_from_z(Vec3 z)
{
    z = vec3_norm(z);
    Vec3 up = (absf(z.z) < (1.f - EPSILON)) ? vec3(0, 0, 1.0f) : vec3(1.0f, 0, 0);
    Vec3 x = vec3_norm(vec3_cross(up, z));
    Vec3 y = vec3_cross(z, x);
    return mat4_vec3(x, y, z, vec3_zero);
}

Mat4 mat4_view(Vec3 a, Rot b)
{
    Mat4 ma = {
        {{0, 0, 1, 0},
         {1, 0, 0, 0},
         {0, 1, 0, 0},
         {0, 0, 0, 1}}};
    return mat4_mul(
        mat4_origin(vec3_neg(a)),
        mat4_mul(mat4_inverse_rot(b), ma));
}

Mat4 mat4_transform(Vec3 origin, float scale)
{
    Mat4 mt = {
        scale, 0, 0, 0,
        0, scale, 0, 0,
        0, 0, scale, 0,
        origin.x, origin.y, origin.z, 1};
    return mt;
}

float mat4_det(Mat4 m)
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

Mat4 mat4_inv(Mat4 m)
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