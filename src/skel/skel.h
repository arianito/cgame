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

void update_matrices(Skel *self);
void bone_upd_world_pos(Skel *self, int bone, Vec2 pos);
void bone_upd_world_rot(Skel *self, int bone, float rotation);
void bone_upd_world_scale(Skel *self, int bone, Vec2 scale);
void bone_upd_local_rot(Skel *self, int bone, float rotation);