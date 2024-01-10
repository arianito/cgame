#include "mat4.h"

#include <immintrin.h>
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