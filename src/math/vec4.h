#ifndef cgame_VEC4_H
#define cgame_VEC4_H

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} Vec4;

static const Vec4 vec4_zero = {0, 0, 0, 0};
static const Vec4 vec4_one = {1, 1, 1, 1};

#define vec4(a, b, c, d) ((Vec4){a, b, c, d})
#define vec4f(a) ((Vec4){a, a, a, a})

#endif