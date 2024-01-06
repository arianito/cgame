#ifndef cgame_TRIANGULATE_H
#define cgame_TRIANGULATE_H

#include "math/triangle.h"
#include "math/edge.h"
#include "math/vec3.h"
#include "adt/murmur.h"
#include "adt/fastset.h"
#include "adt/fastmap.h"
#include "adt/fastvec.h"

inline static bool __compare_Vec3(Vec3 a, Vec3 b)
{
    return vec3_near_eq(a, b);
}

inline static uint64_t __hashof_Vec3(Vec3 key, uint64_t seed)
{
    char bytes[12];
    memcpy(bytes, &key, 12);
    return murmurhash(bytes, 12, seed);
}

inline static int __compare_Edge(Edge a, Edge b)
{
    return edge_near_eq(a, b);
}

inline static uint64_t __hashof_Edge(Edge e, int seed)
{
    return __hashof_Vec3(e.a, seed) ^ __hashof_Vec3(e.b, seed);
}

make_fastset_directives(Edge, Edge, __compare_Edge, __hashof_Edge);

void tri_convex_hull(Vec3 points[], int n, Vec3 hull[], int *m);
void tri_exclude_triangle_from_tris(Triangle out_tris[], int *out_n, Triangle supra);
void tri_exclude_triangle_from_edges(Edge edges[], int *out_n, Triangle t);
void tri_delaunay_triangulate(Triangle supra, Vec3 vertices[], int vn, Triangle tris[], int *out_n);
void tri_unique_edges(Triangle tris[], int nt, Fastset_Edge *edges);
void tri_prims_mst(Vec3 begin, Fastset_Edge *edges, Edge out_edges[], int *out_n);

#endif