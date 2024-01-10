#pragma once

#include "math/mat3.h"
#include "math/rot2.h"
#include "math/vec2.h"
#include "engine/string.h"

typedef struct Bone
{
    StrView name;
    
    Vec2 position;
    float rotation;
    float len;
    Vec2 scale;
    Vec2 shear;
    int type;

    int parent;
    Mat3 world;
    Mat3 local;
} Bone;

typedef struct
{
    void* context;
} Skel;
