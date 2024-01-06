#ifndef cgame_DEBUG_H
#define cgame_DEBUG_H

#include "math/vec2.h"
#include "math/vec3.h"
#include "math/rot.h"
#include "math/color.h"

void debug_init();

void debug_render();

void debug_terminate();

void debug_color(Color color);

void debug_reset();

void debug_origin(Vec2 origin);

void debug_rotation(Rot rot);

void debug_scale(float scale);

void debug_string(Vec2 pos, const char *s, int n);

void debug_string3d(Vec3 pos, const char *str, int n);

void debug_stringf(Vec2 pos, const char *fmt, ...);

void debug_string3df(Vec3 pos, const char *fmt, ...);

#endif