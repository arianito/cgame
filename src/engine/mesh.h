#ifndef cgame_MESH_H
#define cgame_MESH_H

#include "math/vec3.h"
#include "math/vec2.h"
#include "adt/fastvec.h"
#include "adt/fastmap.h"
#include "adt/common.h"

typedef struct
{
    Vec3 position;
    Vec3 normal;
    Vec2 coords;
} MeshVertex;

make_fastvec_directives(MeshVertex, MeshVertex);
make_fastvec_directives(MeshIndices, int32_t);

typedef struct
{
    Fastvec_MeshVertex *vertices;
    Fastvec_MeshIndices *indices;
} RawMesh;

RawMesh *mesh_raw_from_obj(const char *p);

void *mesh_raw_free(RawMesh *ptr);

typedef int32_t MeshId;

typedef struct
{
    MeshId id;
    const char *name;
    uint32_t vao;
    uint32_t vbo;
    uint32_t ebo;
    uint32_t length;
} Mesh;

make_fastmap_directives(StrMeshId, const char *, MeshId, adt_compare_string, adt_hash_string);
make_fastvec_directives(Mesh, Mesh);

void mesh_init();

Mesh *mesh_load(const char *name, const char *p);

Mesh *mesh_get_byname(const char *name);

Mesh *mesh_get(int id);

bool mesh_has(int id);

void mesh_clear();

void mesh_destroy();

#endif