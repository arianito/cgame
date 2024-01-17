#pragma once

#include "adt/str.h"
#include "math/vec2.h"

#define ANIM_PROP_NONE 0
#define ANIM_PROP_X 1
#define ANIM_PROP_Y 2
#define ANIM_PROP_ROTATION 3

#define ANIM_SEQ_STATE_NONE 0
#define ANIM_SEQ_STATE_EXPAND 1

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
    int state0;
    float scale0;
} AnimSequence;

typedef struct
{
    StrView name;
    AnimSequence *data;
    int length;
} Anim;

float anim_iterpolate(AnimSequence *seq, float time);

void anim_control_points(KeyFrame *pkf, KeyFrame *kf, Vec2 qs[4]);