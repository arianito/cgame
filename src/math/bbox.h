#ifndef cgame_BBOX_H
#define cgame_BBOX_H

#include "scalar.h"
#include "defs.h"
#include "vec3.h"
#include "edge.h"

typedef struct
{
    Vec3 min;
    Vec3 max;
} BBox;
#define bbox(a, b) ((BBox){a, b})

static const BBox bbox_empty = {{MAX_FLOAT, MAX_FLOAT, MAX_FLOAT}, {MIN_FLOAT, MIN_FLOAT, MIN_FLOAT}};
static const BBox bbox_zero = {{0, 0, 0}, {0, 0, 0}};

inline static float bbox_depth(BBox b) { return (b.max.x - b.min.x); }
inline static float bbox_height(BBox b) { return (b.max.z - b.min.z); }
inline static float bbox_width(BBox b) { return (b.max.y - b.min.y); }

inline static void bbox_vertices(BBox b, Vec3 *vertices)
{
    vertices[0] = b.min;
    vertices[1] = vec3(b.min.x, b.max.y, b.min.z);
    vertices[2] = vec3(b.max.x, b.max.y, b.min.z);
    vertices[3] = vec3(b.max.x, b.min.y, b.min.z);
    vertices[4] = vec3(b.min.x, b.min.y, b.max.z);
    vertices[5] = vec3(b.min.x, b.max.y, b.max.z);
    vertices[6] = b.max;
    vertices[7] = vec3(b.max.x, b.min.y, b.max.z);
}

inline static void bbox_edges(BBox b, Edge *edges)
{
    Vec3 vertices[8];
    bbox_vertices(b, vertices);

    edges[0] = edge(vertices[0], vertices[1]);
    edges[1] = edge(vertices[1], vertices[2]);
    edges[2] = edge(vertices[2], vertices[3]);
    edges[3] = edge(vertices[3], vertices[0]);

    edges[4] = edge(vertices[4], vertices[5]);
    edges[5] = edge(vertices[5], vertices[6]);
    edges[6] = edge(vertices[6], vertices[7]);
    edges[7] = edge(vertices[7], vertices[4]);

    edges[8] = edge(vertices[0], vertices[4]);
    edges[9] = edge(vertices[1], vertices[5]);
    edges[10] = edge(vertices[2], vertices[6]);
    edges[11] = edge(vertices[3], vertices[7]);
}

inline static BBox bbox_project_xy(BBox b)
{
    b.min.z = 0;
    b.max.z = 0;
    return b;
}

inline static BBox bbox_project_xz(BBox b)
{
    b.min.y = 0;
    b.max.y = 0;
    return b;
}

inline static BBox bbox_project_yz(BBox b)
{
    b.min.x = 0;
    b.max.x = 0;
    return b;
}

inline static BBox bbox_project_axis(BBox b, UnitAxisEnum axis)
{
    if (axis == 0)
    {
        b.min.x = 0;
        b.max.x = 0;
    }
    else if (axis == 1)
    {
        b.min.y = 0;
        b.max.y = 0;
    }
    else if (axis == 2)
    {
        b.min.z = 0;
        b.max.z = 0;
    }
    return b;
}

inline static Vec3 bbox_center(BBox b)
{
    Vec3 a;
    a.x = (b.max.x + b.min.x) * 0.5f;
    a.y = (b.max.y + b.min.y) * 0.5f;
    a.z = (b.max.z + b.min.z) * 0.5f;
    return a;
}

inline static Vec3 bbox_size(BBox b)
{
    Vec3 a;
    a.x = (b.max.x - b.min.x);
    a.y = (b.max.y - b.min.y);
    a.z = (b.max.z - b.min.z);
    return a;
}

inline static BBox bbox_snap(BBox b, float s)
{
    b.min = vec3_snap(b.min, s);
    b.max = vec3_snap(b.max, s);
    return b;
}

inline static BBox bbox_snap_cube(BBox b, float s)
{
    b.min = vec3_snap_cube(b.min, s);
    b.max = vec3_snap_cube(b.max, s);
    return b;
}

inline static float bbox_area(BBox b)
{
    return maxf(0, b.max.x - b.min.x) * maxf(0, b.max.y - b.min.y) * maxf(0, b.max.z - b.min.z);
}

inline static float bbox_margin(BBox b)
{
    return maxf(0, b.max.x - b.min.x) + maxf(0, b.max.y - b.min.y) + maxf(0, b.max.z - b.min.z);
}

inline static BBox bbox_expand(BBox b, float offset)
{
    b.min.x -= offset;
    b.min.y -= offset;
    b.min.z -= offset;
    b.max.x += offset;
    b.max.y += offset;
    b.max.z += offset;
    return b;
}

inline static BBox bbox_extend(BBox a, BBox b)
{
    a.min = vec3_min(a.min, b.min);
    a.max = vec3_max(a.max, b.max);
    return a;
}

inline static BBox bbox_intersection(BBox a, BBox b)
{
    a.min = vec3_max(a.min, b.min);
    a.max = vec3_min(a.max, b.max);
    return a;
}

inline static int bbox_contains(BBox a, BBox b)
{
    return a.min.x <= b.min.x &&
           a.min.y <= b.min.y &&
           a.min.z <= b.min.z &&
           a.max.x >= b.max.x &&
           a.max.y >= b.max.y &&
           a.max.z >= b.max.z;
}

inline static int bbox_contains_point(BBox a, Vec3 b)
{
    return a.min.x <= b.x &&
           a.min.y <= b.y &&
           a.min.z <= b.z &&
           a.max.x >= b.x &&
           a.max.y >= b.y &&
           a.max.z >= b.z;
}

inline static int bbox_intersects(BBox a, BBox b)
{
    return a.min.x <= b.max.x &&
           a.min.y <= b.max.y &&
           a.min.z <= b.max.z &&
           a.max.x >= b.min.x &&
           a.max.y >= b.min.y &&
           a.max.z >= b.min.z;
}

inline static float axisDist(float p, float min, float max)
{
    return p < min ? min - p : p > max ? p - max
                                       : 0;
}

inline static float bbox_distance(BBox a, Vec3 b)
{
    float dX = axisDist(b.x, a.min.x, a.max.x);
    float dY = axisDist(b.y, a.min.y, a.max.y);
    float dZ = axisDist(b.z, a.min.z, a.max.z);
    return sqrf(dX * dX + dY * dY + dZ * dZ);
}

inline static bool bbox_is_plane(BBox b)
{
    return near0f(bbox_width(b)) || near0f(bbox_height(b)) || near0f(bbox_depth(b));
}

inline static bool bbox_is_edge(BBox b)
{
    return (near0f(bbox_width(b)) && near0f(bbox_height(b))) ||
           (near0f(bbox_height(b)) && near0f(bbox_depth(b))) ||
           (near0f(bbox_width(b)) && near0f(bbox_depth(b)));
}

inline static bool bbox_is_point(BBox b)
{
    return near0f(bbox_width(b)) && near0f(bbox_height(b)) && near0f(bbox_depth(b));
}

inline static Vec3 bbox_normal(BBox b)
{
    if (near0f(bbox_width(b)))
        return vec3_right;
    if (near0f(bbox_height(b)))
        return vec3_up;
    if (near0f(bbox_depth(b)))
        return vec3_forward;
    return vec3_zero;
}

inline static BBox bbox_calculate(Vec3 *arr, int n)
{
    if (n == 0)
        return bbox_zero;
    Vec3 min = arr[0];
    Vec3 max = arr[0];
    for (int i = 1; i < n; i++)
    {
        min = vec3_min(min, arr[i]);
        max = vec3_max(max, arr[i]);
    }
    BBox b;
    b.min = min;
    b.max = max;
    return b;
}

inline static BBox bbox_englarge(BBox b, float offset)
{
    b.min = vec3_add(b.min, vec3f(-offset));
    b.max = vec3_add(b.max, vec3f(offset));
    return b;
}

#endif