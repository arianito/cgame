#ifndef cgame_TETRAHEDRON_H
#define cgame_TETRAHEDRON_H

#include "vec3.h"

typedef struct
{
    Vec3 a;
    Vec3 b;
    Vec3 c;
    Vec3 d;
} Tetrahedron;

#define tetrahedron(a, b, c, d) ((Tetrahedron){a, b, c, d})

#endif