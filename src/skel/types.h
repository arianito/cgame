#pragma once

#include "math/mat3.h"
#include "math/rot2.h"
#include "math/vec2.h"
#include "engine/string.h"

typedef enum
{
    SKEL_INHERIT_NONE = 0,
    SKEL_INHERIT_ROTATION = 1,
    SKEL_INHERIT_SCALE = 2,
    SKEL_PREVENT_REFLECTION = 4,
} SkelInh;

typedef enum
{
    SKEL_TYP_NONE = 0,
    SKEL_TYP_ROOT = 1,
    SKEL_TYP_BONE = 2,
    SKEL_TYP_HANDLE = 4
} SkelTyp;

typedef struct Bone
{
    StrView name;

    Vec2 position;
    float rotation;
    float len;
    Vec2 scale;
    Vec2 shear;
    SkelTyp type;
    SkelInh inherit;

    int parent;
    Mat3 world;
    Mat3 local;
    bool dirty;
} Bone;

typedef struct
{
    void *context;
} Skel;
