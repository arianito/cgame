#ifndef cgame_MESH_H
#define cgame_MESH_H

#include "math/vec3.h"
#include "math/vec2.h"
#include "adt/fastvec.h"

typedef struct
{
    Vec3 position;
    Vec3 normal;
    Vec2 coords;
} MeshVertex;

make_fastvec_directives(MeshVertex, MeshVertex);
make_fastvec_directives(MeshIndices, int);

typedef struct
{
    Fastvec_MeshVertex *vertices;
    Fastvec_MeshIndices *indices;
} Mesh;

Mesh *mesh_from_obj(const char *p);

void *mesh_free(Mesh *ptr);

#endif