#ifndef cgame_PLANE_H
#define cgame_PLANE_H

#include "vec3.h"

typedef struct
{
    Vec3 normal;
    float distance;
} Plane;

#define plane(normal, distance) ((Plane){normal, distance})

static inline Plane plane_np(Vec3 normal, Vec3 in_point)
{
    return plane(vec3_norm(normal), -vec3_dot(normal, in_point));
}
static inline Plane plane_vvv(Vec3 a, Vec3 b, Vec3 c)
{
    Vec3 norm = vec3_norm(vec3_cross(vec3_sub(b, a), vec3_sub(c, a)));
    return plane(norm, -vec3_dot(norm, a));
}

static inline Vec3 plane_closest_point(Plane p, Vec3 point)
{
    float pointToPlaneDistance = vec3_dot(p.normal, point) + p.distance;
    return vec3_sub(point, vec3_mulf(p.normal, pointToPlaneDistance));
}

#endif