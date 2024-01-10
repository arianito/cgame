#include "mesh.h"

#include <stdlib.h>
#include <stdio.h>

#include "mem/alloc.h"
#include "file.h"
#include "string.h"

#include "adt/fastvec.h"
#include "glad.h"

make_fastmap_directives(StrMeshId, const char *, MeshId, adt_compare_cstr, adt_hashof_cstr);
make_fastvec_directives(Mesh, Mesh);
make_fastvec_directives(Vec3, Vec3);
make_fastvec_directives(Vec2, Vec2);

RawMesh *mesh_raw_from_obj(const char *p)
{
    size_t cursor = 0;
    StrView line;
    StrView pt = resolve_stack(p);
    FILE *f = fopen(pt.string, "r");
    xxfreestack(pt.string);
    RawMesh *mesh = NULL;
    if (f != NULL)
    {
        Fastvec_Vec3 *positions = fastvec_Vec3_init(2);
        Fastvec_Vec3 *normals = fastvec_Vec3_init(2);
        Fastvec_Vec2 *coords = fastvec_Vec2_init(2);

#define SAFE_RETURN()                \
    fastvec_Vec3_destroy(positions); \
    fastvec_Vec3_destroy(normals);   \
    fastvec_Vec2_destroy(coords);    \
    fclose(f);                       \
    xxfreestack(line.string);

#define CLEAR_MESH() \
    mesh_raw_free(mesh)

        while ((line = readline_stack(f, &cursor)).string != NULL)
        {
            StrView ft = str_first_token(line, ' ');
            if (str_eq(ft, string_const("o")))
            {
                if (mesh != NULL)
                {
                    // do not allow multiple mesh per file
                    SAFE_RETURN();
                    return mesh;
                }
                // alloc mesh when object is found
                mesh = xxmalloc(sizeof(RawMesh));
                mesh->vertices = fastvec_MeshVertex_init(2);
                mesh->indices = fastvec_MeshIndices_init(2);
            }
            else if (str_eq(ft, string_const("v")))
            {
                // load vertex positions
                StrView splits[5];
                int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
                if (n == 3)
                {
                    str_truncate(splits, n);
                    Vec3 p = vec3(str_tofloat(splits[0]), str_tofloat(splits[1]), str_tofloat(splits[2]));
                    fastvec_Vec3_push(positions, p);
                }
            }
            else if (str_eq(ft, string_const("vn")))
            {
                // load normals
                StrView splits[5];
                int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
                if (n == 3)
                {
                    str_truncate(splits, n);
                    Vec3 p = vec3(str_tofloat(splits[0]), str_tofloat(splits[1]), str_tofloat(splits[2]));
                    fastvec_Vec3_push(normals, p);
                }
            }
            else if (str_eq(ft, string_const("vt")))
            {
                // load tex coords
                StrView splits[5];
                int n = str_splitchar(str_last_token(line, ' '), ' ', splits);
                if (n == 2)
                {
                    str_truncate(splits, n);
                    float y = 1 - str_tofloat(splits[1]);
                    Vec2 p = vec2(str_tofloat(splits[0]), y);
                    fastvec_Vec2_push(coords, p);
                }
            }
            else if (str_eq(ft, string_const("f")))
            {

                // generate indices, mesh must be triangulated before import
                StrView face_data[5];
                int n = str_splitchar(str_last_token(line, ' '), ' ', face_data);

                for (int i = 0; i < n; i++)
                {
                    StrView splits[5];
                    int m = str_splitchar(face_data[i], '/', splits);

                    if (m != 3)
                    {
                        SAFE_RETURN();
                        CLEAR_MESH();
                        printf("mesh: invalid indices size\n");
                        return NULL;
                    }
                    str_truncate(splits, m);

                    long i0 = str_tolong(splits[0]);
                    long i1 = str_tolong(splits[1]);
                    long i2 = str_tolong(splits[2]);

                    MeshVertex vert;
                    vert.position = positions->vector[i0 < 0 ? (positions->length + i0) : (i0 - 1)];
                    vert.coords = coords->vector[i1 < 0 ? (coords->length + i1) : (i1 - 1)];
                    vert.normal = normals->vector[i2 < 0 ? (normals->length + i2) : (i2 - 1)];
                    fastvec_MeshVertex_push(mesh->vertices, vert);
                }
                if (mesh->vertices->length == 0)
                {
                    SAFE_RETURN();
                    CLEAR_MESH();
                    printf("mesh: no vertices to create mesh\n");
                    return NULL;
                }
                if (n == 3)
                {
                    int indices[] = {0, 1, 2};
                    for (int i = 0; i < 3; i++)
                    {
                        int ind = (mesh->vertices->length - n) + indices[i];
                        fastvec_MeshIndices_push(mesh->indices, ind);
                    }
                }
            }
            xxfreestack(line.string);
        }
        SAFE_RETURN();
    }
    return mesh;
}

void *mesh_raw_free(RawMesh *ptr)
{
    if (ptr != NULL)
    {
        fastvec_MeshVertex_destroy(ptr->vertices);
        fastvec_MeshIndices_destroy(ptr->indices);
        xxfree(ptr, sizeof(RawMesh));
    }
}

typedef struct
{
    Fastmap_StrMeshId *indices;
    Fastvec_Mesh *meshes;
} MeshContext;

static MeshContext *self;

void mesh_init()
{
    self = (MeshContext *)xxarena(sizeof(MeshContext));
    self->indices = fastmap_StrMeshId_init();
    self->meshes = fastvec_Mesh_init(2);
}

Mesh *mesh_load(const char *name, const char *p)
{
    FastmapNode_StrMeshId *node = fastmap_StrMeshId_get(self->indices, name);
    if (node != NULL)
        return &self->meshes->vector[node->value];

    Mesh mesh;
    mesh.name = name;
    mesh.id = self->indices->length;

    RawMesh *raw = mesh_raw_from_obj(p);

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, raw->vertices->length * sizeof(MeshVertex), raw->vertices->vector, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, raw->indices->length * sizeof(int), raw->indices->vector, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void *)offsetof(MeshVertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void *)offsetof(MeshVertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void *)offsetof(MeshVertex, coords));
    glBindVertexArray(0);

    mesh.length = raw->indices->length;

    mesh_raw_free(raw);

    fastvec_Mesh_push(self->meshes, mesh);
    node = fastmap_StrMeshId_put(self->indices, name);
    node->value = mesh.id;
    return &self->meshes->vector[mesh.id];
}

Mesh *mesh_get_byname(const char *name)
{
    FastmapNode_StrMeshId *node = fastmap_StrMeshId_get(self->indices, name);
    if (node == NULL)
        return NULL;
    return &self->meshes->vector[node->value];
}

Mesh *mesh_get(MeshId id)
{
    return &self->meshes->vector[id];
}

bool mesh_has(MeshId id)
{
    return id >= 0 && id < self->meshes->length;
}

void mesh_clear()
{
    for (int i = 0; i < self->meshes->length; i++)
    {
        Mesh *m = &self->meshes->vector[i];
        glDeleteVertexArrays(1, &m->vao);
        glDeleteBuffers(1, &m->vbo);
        glDeleteBuffers(1, &m->ebo);
    }
    fastmap_StrMeshId_clear(self->indices);
    fastvec_Mesh_clear(self->meshes);
}

void mesh_destroy()
{
    for (int i = 0; i < self->meshes->length; i++)
    {
        Mesh *m = &self->meshes->vector[i];
        glDeleteVertexArrays(1, &m->vao);
        glDeleteBuffers(1, &m->vbo);
        glDeleteBuffers(1, &m->ebo);
    }
    fastmap_StrMeshId_destroy(self->indices);
    fastvec_Mesh_destroy(self->meshes);
}
