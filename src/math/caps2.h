#pragma once
#include "vec2.h"

typedef struct
{
    Vec2 a;
    Vec2 b;
    float r;
} Caps2;

#define caps2(a, b, r) ((Caps2){a, b, r})
