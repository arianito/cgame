#ifndef cgame_MESH_H
#define cgame_MESH_H

#include "math/vec3.h"
#include "math/vec2.h"

typedef struct
{
    Vec3 position;
    Vec3 normal;
    Vec2 coords;
} MeshVertex;


typedef struct
{
    MeshVertex *vertices;
    int *indices;
    int lenght;
} Mesh;


Mesh* mesh_from_obj(const char* p);

void* mesh_free(Mesh* ptr);

#endif