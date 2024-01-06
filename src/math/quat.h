#ifndef cgame_QUAT_H
#define cgame_QUAT_H

#include "vec3.h"

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} Quat;

static const Quat quat_identity = {0, 0, 0, 1};

#define quat(x, y, z, w) ((Quat){x, y, z, w})

Quat quat_mul(Quat a, Quat b);
Quat quat_mulf(Quat a, float b);
float quat_dot(Quat a, Quat b);
float quat_sqr_length(Quat a);
float quat_length(Quat a);
Quat quat_normalize(Quat a);
Vec3 quat_unrotate(Quat q, Vec3 b);
Vec3 quat_rotate(Quat q, Vec3 b);
Vec3 quat_forward(Quat q);
Vec3 quat_right(Quat q);
Vec3 quat_up(Quat q);

#endif