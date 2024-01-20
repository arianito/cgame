#pragma once

#include "adt/str.h"
#include "math/vec2.h"
#include "math/color.h"

#define ANIM_PROP_NONE 0
#define ANIM_PROP_X 1
#define ANIM_PROP_Y 2
#define ANIM_PROP_ROTATION 3
#define ANIM_PROP_SCALE_X 4
#define ANIM_PROP_SCALE_Y 5

static const StrView anim_property_names[] = {
    str("UNKNOWN"),
    str("X"),
    str("Y"),
    str("ROTATION"),
    str("SCALE X"),
    str("SCALE Y"),
};

static const Color anim_property_colors[] = {
    rgb(0, 0, 0),
    rgb(255,123,123),
    rgb(255,82,82),
    rgb(80, 141, 105),
    rgb(11, 96, 176),
    rgb(64, 162, 216),
};

typedef struct
{
    float t;
    float value;
    float cubic[4];
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
    float min0;
    float max0;
} AnimSequence;

typedef struct
{
    StrView name;
    AnimSequence *data;
    int length;
} Anim;

float anim_iterpolate(AnimSequence *seq, float time);

void anim_control_points(KeyFrame *pkf, KeyFrame *kf, Vec2 qs[4]);
KeyFrame *anim_find(AnimSequence *seq, float time, float epsilon);