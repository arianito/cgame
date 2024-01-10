#include "mat3.h"

#include <immintrin.h>

Mat3 mat3_mul(Mat3 a, Mat3 b)
{
    Mat3 m;
#if defined(USE_AVX_256) || defined(USE_AVX_128)
    __m128 tmp, sum;
    __m128 b0 = _mm_loadu_ps(b.m[0]);
    __m128 b1 = _mm_loadu_ps(b.m[1]);
    __m128 b2 = _mm_loadu_ps(b.m[2]);

    tmp = _mm_set_ps1(a.m[0][0]);
    sum = _mm_mul_ps(tmp, b0);
    tmp = _mm_set_ps1(a.m[0][1]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b1), sum);
    tmp = _mm_set_ps1(a.m[0][2]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b2), sum);
    _mm_storeu_ps(m.m[0], sum);

    tmp = _mm_set_ps1(a.m[1][0]);
    sum = _mm_mul_ps(tmp, b0);
    tmp = _mm_set_ps1(a.m[1][1]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b1), sum);
    tmp = _mm_set_ps1(a.m[1][2]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b2), sum);
    _mm_storeu_ps(m.m[1], sum);

    tmp = _mm_set_ps1(a.m[2][0]);
    sum = _mm_mul_ps(tmp, b0);
    tmp = _mm_set_ps1(a.m[2][1]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b1), sum);
    tmp = _mm_set_ps1(a.m[2][2]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b2), sum);
    _mm_storeu_ps(m.m[2], sum);

    tmp = _mm_set_ps1(a.m[3][0]);
    sum = _mm_mul_ps(tmp, b0);
    tmp = _mm_set_ps1(a.m[3][1]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b1), sum);
    tmp = _mm_set_ps1(a.m[3][2]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b2), sum);
    _mm_storeu_ps(m.m[3], sum);
#else
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            m.m[i][j] = 0;
            for (int k = 0; k < 3; ++k)
            {
                m.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
#endif
    return m;
}

Vec3 mat3_mulv3(Mat3 a, Vec3 b)
{
#if defined(USE_AVX_256) || defined(USE_AVX_128)

    __m128 tmp, sum;
    __m128 b0 = _mm_loadu_ps(a.m[0]);
    __m128 b1 = _mm_loadu_ps(a.m[1]);
    __m128 b2 = _mm_loadu_ps(a.m[2]);

    tmp = _mm_set_ps1(b.x);
    sum = _mm_mul_ps(tmp, b0);
    tmp = _mm_set_ps1(b.y);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b1), sum);
    tmp = _mm_set_ps1(b.z);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b2), sum);
    _mm_storeu_ps((float *)(&b), sum);
    return b;

#else
#endif
    return b;
}

Vec2 mat3_mulv2(Mat3 a, Vec2 b, float w)
{
    Vec3 v = {b.x, b.y, w};
    v = mat3_mulv3(a, v);
    b.x = v.x;
    b.y = v.y;
    return b;
}
