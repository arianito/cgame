#ifndef cgame_SPHERE_H
#define cgame_SPHERE_H

#include "vec3.h"

typedef struct
{
    Vec3 position;
    float radius;
} Sphere;

#define sphere(pos, r) ((Sphere){pos, r})

#endif