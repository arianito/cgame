#ifndef cgame_ROT2_H
#define cgame_ROT2_H

#include "scalar.h"
#include "vec2_ifc.h"

typedef struct
{
    float cos;
    float sin;
} Rot2;

static const Rot2 rot2_identity = {1, 0};

#define rot2(cos, sin) ((Rot2){cos, sin})
#define rot2f(deg) (rot2(cosdf(deg), sindf(deg)))

inline static float rot2_angle(Rot2 q) { return atan2df(q.sin, q.cos); }
inline static Vec2 rot2_axis_x(Rot2 q) { return vec2(q.cos, q.sin); }
inline static Vec2 rot2_axis_y(Rot2 q) { return vec2(-q.sin, q.cos); }
inline static Rot2 rot2_mul(Rot2 q, Rot2 r) { return rot2(q.cos * r.cos - q.sin * r.sin, q.sin * r.cos + q.cos * r.sin); }
inline static Rot2 rot2_unmul(Rot2 q, Rot2 r) { return rot2(q.cos * r.cos + q.sin * r.sin, q.cos * r.sin - q.sin * r.cos); }
inline static Vec2 rot2_rotate(Rot2 q, Vec2 vec) { return vec2(q.cos * vec.x - q.sin * vec.y, q.sin * vec.x + q.cos * vec.y); }
static inline Vec2 rot2_unrotate(Rot2 q, Vec2 vec) { return vec2(q.cos * vec.x + q.sin * vec.y, -q.sin * vec.x + q.cos * vec.y); }
#endif