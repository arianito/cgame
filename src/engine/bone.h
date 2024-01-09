#pragma once

#include "math/rot2.h"
#include "math/vec2.h"
#include "adt/fastvec.h"

typedef struct
{
    Vec2 position;
    float angle;
    float len;
    float len0;
} Bone2d;

make_fastvec_directives(Bone2d, Bone2d);

typedef struct
{
    void* context;
    Fastvec_Bone2d *bones;
    Vec2 origin;
    Vec2 target;
    bool init;
} Skeleton2d;

Skeleton2d *skeleton_cerate(Vec2 pos);
void skeleton_add(Skeleton2d *self, Vec2 pos);
void skeleton_step(Skeleton2d *self, float dt);
void skeleton_free(Skeleton2d *self);