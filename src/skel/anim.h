#pragma once

#include "adt/str.h"
#include "math/vec2.h"

typedef struct
{
    float t;
    float value;
} KeyFrame;

typedef struct
{
    int property;
    int length;
    KeyFrame *frames;
} PropertyKeyFrame;

typedef struct
{
    int bone;
    int length;
    PropertyKeyFrame *properties;
} BoneKeyFrame;

typedef struct
{
    StrView name;
    float duration;
    bool loop;

    BoneKeyFrame *bones;
} Anim;
