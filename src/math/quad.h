#ifndef cgame_QUAD_H
#define cgame_QUAD_H

#include "vec3.h"
#include "rot.h"

typedef struct
{
    Vec3 a;
    Vec3 b;
    Vec3 c;
    Vec3 d;
} Quad;

static const Quad quad_zero = {vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0)};

#define quad(x, y, z, w) ((Quad){x, y, z, w})

Quad quad_rot(Quad q, Rot r);
Quad quad_offset(Quad q, Vec3 b);

#endif