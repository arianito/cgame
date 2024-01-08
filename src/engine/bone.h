#pragma once

#include "math/rot2.h"
#include "math/vec2.h"
#include "adt/fastvec.h"

typedef struct
{
    Vec2 position;
    Vec2 direction;
    Vec2 target;
    bool attached;
} Bone2d;

make_fastvec_directives(Bone2d, Bone2d);

typedef struct
{
    Fastvec_Bone2d *bones;
} Skeleton2d;

Skeleton2d *skeleton_cerate(Vec2 pos);
void skeleton_add(Skeleton2d *self, Vec2 pos);
void skeleton_step(Skeleton2d *self, float dt);
void skeleton_free(Skeleton2d *self);