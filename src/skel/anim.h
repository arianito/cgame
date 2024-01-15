#pragma once

#include "adt/str.h"
#include "math/vec2.h"

#define ANIM_PROP_NONE 0
#define ANIM_PROP_X 1
#define ANIM_PROP_Y 2
#define ANIM_PROP_ROTATION 3

static const StrView anim_property_names[] = {
    str("UNKNOWN"),
    str("X"),
    str("Y"),
    str("ROTATION"),
};

typedef struct
{
    float t;
    float value;
} KeyFrame;

typedef struct
{
    int id;
    int type;
    StrView name;
    KeyFrame *frames;
    int length;
    // prv
    int i0;
} AnimSequence;

typedef struct
{
    StrView name;
    AnimSequence *data;
    int length;
} Anim;
