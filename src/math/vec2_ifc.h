#ifndef cgame_VEC2_IFC_H
#define cgame_VEC2_IFC_H

#include "defs.h"

typedef struct
{
    float x;
    float y;
} Vec2;

static const Vec2 vec2_inf = {MAX_FLOAT, MAX_FLOAT};
static const Vec2 vec2_zero = {0, 0};
static const Vec2 vec2_one = {1, 1};
static const Vec2 vec2_up = {0, 1};
static const Vec2 vec2_down = {0, -1};
static const Vec2 vec2_left = {-1, 0};
static const Vec2 vec2_right = {1, 0};

#define vec2(a, b) ((Vec2){a, b})
#define vec2f(a) ((Vec2){a, a})

#endif