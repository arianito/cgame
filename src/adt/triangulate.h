#ifndef cgame_TRIANGULATE_H
#define cgame_TRIANGULATE_H

#include "engine/mathf.h"
#include "custom_types.h"

void tri_convex_hull(Vec3 points[], int n, Vec3 hull[], int *m);
void tri_exclude_triangle_from_tris(Triangle out_tris[], int *out_n, Triangle supra);
void tri_exclude_triangle_from_edges(Edge edges[], int *out_n, Triangle t);
void tri_delaunay_triangulate(Triangle supra, Vec3 vertices[], int vn, Triangle tris[], int *out_n);
void tri_unique_edges(Triangle tris[], int nt, Fastset_Edge *edges);
void tri_prims_mst(Vec3 begin, Fastset_Edge *edges, Edge out_edges[], int *out_n);


#endif