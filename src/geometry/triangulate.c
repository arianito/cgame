#include "triangulate.h"
#include "mem/alloc.h"
#include "math/vec3.h"
#include "math/edge.h"

make_fastset_directives(Vec3, Vec3, adt_compare_vec3, adt_hashof_vec3);
make_fastvec_directives(Edge, Edge);

static int orientation(Vec3 p, Vec3 q, Vec3 r)
{
    int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
    if (val == 0)
        return 0;
    return (val > 0) ? 1 : -1;
}

static void get_bad_triangles(Triangle triangles[], int n, Vec3 p, Triangle out_tris[], int *out_n)
{
    int b = 0;
    for (int i = 0; i < n; i++)
        if (triangle_circumfere(triangles[i], p))
            out_tris[b++] = triangles[i];
    *out_n = b;
}

static void exclude_bad_triangles(Triangle out_tris[], int *n, Triangle bad[], int m)
{
    int l = 0;
    for (int i = 0; i < *n; i++)
    {
        int ok = 1;
        for (int j = 0; j < m; j++)
        {
            if (triangle_near_eq(out_tris[i], bad[j]))
            {
                ok = 0;
                break;
            }
        }
        if (ok)
            out_tris[l++] = out_tris[i];
    }
    *n = l;
}

static int contains_edge(int current, Triangle bad[], int m, Edge edge)
{
    for (int i = 0; i < m; i++)
        if (i != current && triangle_has_edge(bad[i], edge))
            return 0;
    return 1;
}

void tri_convex_hull(Vec3 points[], int n, Vec3 hull[], int *out_m)
{
    if (n < 3)
        return;
    *out_m = 0;
    int l = 0;
    for (int i = 1; i < n; i++)
    {
        if (points[i].x < points[l].x)
            l = i;
    }
    int p = l, q;
    do
    {
        hull[(*out_m)++] = points[p];
        q = (p + 1) % n;
        for (int i = 0; i < n; i++)
        {
            if (orientation(points[p], points[i], points[q]) == -1)
                q = i;
        }
        p = q;
    } while (p != l);
}

void tri_exclude_triangle_from_tris(Triangle out_tris[], int *out_n, Triangle supra)
{
    int l = 0;
    for (int i = 0; i < *out_n; i++)
        if (!triangle_has_vertex(out_tris[i], supra.a) && !triangle_has_vertex(out_tris[i], supra.b) && !triangle_has_vertex(out_tris[i], supra.c))
            out_tris[l++] = out_tris[i];
    *out_n = l;
}
void tri_exclude_triangle_from_edges(Edge out_edges[], int *out_n, Triangle supra)
{
    int l = 0;
    for (int i = 0; i < *out_n; i++)
        if (!edge_has_vertex(out_edges[i], supra.a) && !edge_has_vertex(out_edges[i], supra.b) && !edge_has_vertex(out_edges[i], supra.c))
            out_edges[l++] = out_edges[i];
    *out_n = l;
}

void tri_delaunay_triangulate(Triangle supra, Vec3 vertices[], int vn, Triangle out_tris[], int *out_n)
{
    Triangle bad[32];
    int tn = 1;
    out_tris[0] = supra;

    for (int v = 0; v < vn; v++)
    {
        int bn = 0;
        get_bad_triangles(out_tris, tn, vertices[v], bad, &bn);
        for (int i = 0; i < bn; i++)
        {
            Edge edges[3];
            triangle_edges(bad[i], edges);
            for (int e = 0; e < 3; e++)
                if (contains_edge(i, bad, bn, edges[e]))
                    out_tris[tn++] = triangle(edges[e].a, edges[e].b, vertices[v]);
        }
        exclude_bad_triangles(out_tris, &tn, bad, bn);
    }
    *out_n = tn;
}

void tri_unique_edges(Triangle tris[], int nt, Fastset_Edge *edges)
{
    for (int i = 0; i < nt; i++)
    {
        fastset_Edge_put(edges, edge(tris[i].a, tris[i].b));
        fastset_Edge_put(edges, edge(tris[i].b, tris[i].c));
        fastset_Edge_put(edges, edge(tris[i].c, tris[i].a));
    }
}

void tri_prims_mst(Vec3 begin, Fastset_Edge *edges, Edge out_edges[], int *out_n)
{
    if (edges->length == 0)
        return;
    int m = 0;
    Fastset_Vec3 *vertices = fastset_Vec3_init();
    fastset_Vec3_put(vertices, begin);

    while (vertices->length > 0)
    {
        int found = 0;
        Edge targetEdge;
        float minLength = MAX_FLOAT;

        fastset_for(Edge, edges, it)
        {
            Edge e = it.node->key;
            int a1 = fastset_Vec3_get(vertices, e.a) != NULL;
            int a2 = fastset_Vec3_get(vertices, e.b) != NULL;

            if (a1 != a2)
            {
                float len = vec3_dist(e.a, e.b);
                if (len < minLength)
                {
                    found = 1;
                    targetEdge = e;
                    minLength = len;
                }
            }
        }

        if (!found)
            break;

        out_edges[m++] = targetEdge;
        fastset_Edge_remove(edges, targetEdge);
        fastset_Vec3_put(vertices, targetEdge.a);
        fastset_Vec3_put(vertices, targetEdge.b);
    }
    fastset_Vec3_destroy(vertices);
    *out_n = m;
}