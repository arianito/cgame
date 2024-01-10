#pragma once

#include "types.h"

Skel *skeleton_cerate(Vec2 pos);
void skeleton_add(Skel *self, Vec2 pos);
void skeleton_step(Skel *self, float dt);
void skeleton_loadfile(Skel *self, const char* path);
void skeleton_free(Skel *self);
void skeleton_render(Skel *self);