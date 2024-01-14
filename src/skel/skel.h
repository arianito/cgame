#pragma once


#include "math/vec2.h"
#include "math/mat3.h"

typedef struct
{
    void *context;
} Skel;

Skel *skeleton_cerate(Vec2 pos);
void skeleton_step(Skel *self, float dt);
void skeleton_loadfile(Skel *self, const char* path);
void skeleton_destroy(Skel *self);
void skeleton_render(Skel *self);

void bone_set_wpos(Skel *self, int bone, Vec2 pos);
void bone_set_wrot(Skel *self, int bone, float rotation);
void bone_set_wscale(Skel *self, int bone, Vec2 scale);

void bone_set_lpos(Skel *self, int bone, Vec2 pos);
void bone_set_lrot(Skel *self, int bone, float rotation);
void bone_set_lscale(Skel *self, int bone, Vec2 scale);