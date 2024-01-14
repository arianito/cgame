#pragma once

#include "adt/str.h"
#include "math/vec2.h"


typedef struct
{
    float value;
    float t;
} KeyFrame;

typedef struct PropertyKeyFrame
{
    int property;
    int length;
    KeyFrame *frames;
};

typedef struct BoneKeyFrame
{
    int bone;
    int length;
    PropertyKeyFrame *properties;
};


typedef struct
{
    StrView name;
    float duration;
    bool loop;

    BoneKeyFrame *bones;
} Anim;
