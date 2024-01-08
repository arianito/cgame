#include "mesh.h"

#include <stdlib.h>
#include <stdio.h>

#include "mem/alloc.h"
#include "file.h"
#include "string.h"

#include "adt/fastvec.h"

make_fastvec_directives(Vec3, Vec3);
make_fastvec_directives(Vec2, Vec2);

inline static StrView first_token(StrView line)
{
    if (str_empty(line))
        return str_null;
    int32_t start = str_skipchar(line, ' ', 0);
    int32_t end = str_untilchar(line, ' ', start);
    if (start != -1 && end != -1)
        return str_substr(line, start, end - start);
    return str_null;
}

inline static StrView last_token(StrView line)
{
    if (str_empty(line))
        return str_null;
    int32_t start = str_skipchar(line, ' ', 0);
    int32_t next = str_untilchar(line, ' ', start);
    start = str_skipchar(line, ' ', next);
    next = str_untilchar_rev(line, ' ', 0);
    if (start != -1 && next != -1)
        return str_substr(line, start, next - start);
    if (start != -1)
        return str_substr(line, start, 0);
    return str_null;
}

Mesh *mesh_from_obj(const char *p)
{
    size_t cursor = 0;
    StrView line;
    StrView pt = resolve_stack(p);
    FILE *f = fopen(pt.string, "r");
    xxfreestack(pt.string);

    Mesh *mesh = NULL;
    
    StrView dbg = str_substr(string_const("hello world"), 0, 2);
    char* tmp = str_tostack(dbg);
    printf("|%s|%d| \n", tmp, dbg.length);
    xxfreestack(tmp);
    
    

    if (f != NULL)
    {
        Fastvec_Vec3 *positions = fastvec_Vec3_init(2);
        Fastvec_Vec3 *normals = fastvec_Vec3_init(2);
        Fastvec_Vec2 *coords = fastvec_Vec2_init(2);
        while ((line = readline_stack(f, &cursor)).string != NULL)
        {
            StrView ft = first_token(line);
            if (str_eq(ft, string_const("o")))
            {
                if (mesh != NULL)
                {
                    fastvec_Vec3_destroy(positions);
                    fastvec_Vec3_destroy(normals);
                    fastvec_Vec2_destroy(coords);
                    xxfreestack(line.string);
                    fclose(f);
                    return mesh;
                }
                mesh = xxmalloc(sizeof(Mesh));
                mesh->vertices = fastvec_MeshVertex_init(2);
                mesh->indices = fastvec_MeshIndices_init(2);
            }
            else if (str_eq(ft, string_const("v")))
            {
                StrView pos_data[5];
                int n = str_splitchar(last_token(line), ' ', pos_data);
                if (n == 3)
                {
                    Vec3 p = vec3(str_tofloat(pos_data[0]), str_tofloat(pos_data[1]), str_tofloat(pos_data[2]));
                    fastvec_Vec3_push(positions, p);
                }
            }
            else if (str_eq(ft, string_const("vn")))
            {
                StrView normal_data[5];
                int n = str_splitchar(last_token(line), ' ', normal_data);
                if (n == 3)
                {
                    Vec3 p = vec3(str_tofloat(normal_data[0]), str_tofloat(normal_data[1]), str_tofloat(normal_data[2]));
                    fastvec_Vec3_push(normals, p);
                }
            }
            else if (str_eq(ft, string_const("vt")))
            {
                StrView coord_data[5];
                int n = str_splitchar(last_token(line), ' ', coord_data);
                if (n == 2)
                {
                    Vec2 p = vec2(str_tofloat(coord_data[0]), str_tofloat(coord_data[1]));
                    fastvec_Vec2_push(coords, p);
                }
            }
            else if (str_eq(ft, string_const("f")))
            {

                StrView face_data[5];
                int n = str_splitchar(last_token(line), ' ', face_data);

                for (int i = 0; i < n; i++)
                {
                    StrView index_data[5];
                    int m = str_splitchar(face_data[i], '/', index_data);

                    StrView dbg = face_data[i];
                    char* tmp = str_tostack(dbg);
                    printf("|%s|%d| (%d) \n", tmp, dbg.length, m);
                    xxfreestack(tmp);

                    if (m != 3)
                    {
                        fastvec_Vec3_destroy(positions);
                        fastvec_Vec3_destroy(normals);
                        fastvec_Vec2_destroy(coords);
                        xxfreestack(line.string);
                        fclose(f);
                        mesh_free(mesh);
                        return NULL;
                    }

                    // long i0 = str_tolong(index_data[0]);
                    // long i1 = str_tolong(index_data[1]);
                    // long i2 = str_tolong(index_data[2]);

                    // MeshVertex vert;
                    // vert.position = positions->vector[i0 < 0 ? (positions->length + i0) : (i0 - 1)];
                    // vert.coords = coords->vector[i1 < 0 ? (coords->length + i1) : (i1 - 1)];
                    // vert.normal = normals->vector[i2 < 0 ? (normals->length + i2) : (i2 - 1)];

                    // fastvec_MeshVertex_push(mesh->vertices, vert);
                }
            }
            xxfreestack(line.string);
        }

        fastvec_Vec3_destroy(positions);
        fastvec_Vec3_destroy(normals);
        fastvec_Vec2_destroy(coords);
        fclose(f);
    }
    return mesh;
}

void *mesh_free(Mesh *ptr)
{
    if (ptr != NULL)
    {
        fastvec_MeshVertex_destroy(ptr->vertices);
        fastvec_MeshIndices_destroy(ptr->indices);
        xxfree(ptr, sizeof(Mesh));
    }
}