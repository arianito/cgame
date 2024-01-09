#pragma once

#include "math/rot2.h"
#include "math/vec2.h"
#include "adt/fastvec.h"

typedef struct
{
    Vec2 position;
    float angle;
    float len;
} Bone;

make_fastvec_directives(Bone, Bone);

typedef struct
{
    void* context;
    Fastvec_Bone *bones;
    Vec2 origin;
    Vec2 target;
    bool init;
} Skeleton2d;

Skeleton2d *skeleton_cerate(Vec2 pos);
void skeleton_add(Skeleton2d *self, Vec2 pos);
void skeleton_step(Skeleton2d *self, float dt);
void skeleton_free(Skeleton2d *self);
void skeleton_render(Skeleton2d* self);