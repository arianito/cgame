#pragma once

#include "math/mat3.h"
#include "math/rot2.h"
#include "math/vec2.h"
#include "engine/string.h"
#include "adt/fastvec.h"

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


typedef struct
{
    int index;
    StrView name;

    Vec2 world_position0;
    float world_rotation0;
    Vec2 world_scale0;

    Vec2 world_position;
    float world_rotation;
    Vec2 world_scale;

    Vec2 local_position;
    float local_rotation;
    Vec2 local_scale;

    float len;
    float len0;
    SkelTyp type;
    SkelInh inherit;

    int parent;


    Mat3 world;
    Mat3 local;
} Bone;

