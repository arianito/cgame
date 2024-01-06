#ifndef cgame_MAT2_H
#define cgame_MAT2_H

#include "scalar.h"
#include "vec2_ifc.h"

typedef struct
{
    Vec2 cx;
    Vec2 cy;
} Mat2;

static const Mat2 mat2_identity = {{0, 0}, {0, 0}};
#define mat2(a, b) ((Mat2){a, b})

/// Multiply a 2-by-2 matrix times a 2D vector
static inline Vec2 mat2_mulv(Mat2 A, Vec2 v)
{
    return vec2(
        A.cx.x * v.x + A.cy.x * v.y,
        A.cx.y * v.x + A.cy.y * v.y);
}

/// Get the inverse of a 2-by-2 matrix
static inline Mat2 mat2_inv(Mat2 A)
{
    float a = A.cx.x, b = A.cy.x, c = A.cx.y, d = A.cy.y;
    float det = a * d - b * c;
    if (det != 0.0f)
        det = 1.0f / det;
    return mat2(vec2(det * d, -det * c), vec2(-det * b, det * a));
}

#endif