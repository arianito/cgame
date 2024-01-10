#pragma once


#include "math/vec2.h"
#include "math/mat3.h"

typedef struct
{
    void *context;
} Skel;

Skel *skeleton_cerate(Vec2 pos);
void skeleton_add(Skel *self, Vec2 pos);
void skeleton_step(Skel *self, float dt);
void skeleton_loadfile(Skel *self, const char* path);
void skeleton_free(Skel *self);
void skeleton_render(Skel *self);

float bone_sum_rot(Skel *self, int bone);

Mat3 bone_world_matrix(Skel *self, int bone);
Mat3 bone_local_matrix(Skel *self, int bone);
void bone_upd_world_rot(Skel *self, int bone, float rotation);
void bone_upd_local_rot(Skel *self, int bone, float rotation);