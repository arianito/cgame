#ifndef cgame_TRIANGLE_H
#define cgame_TRIANGLE_H

#include <stdbool.h>

#include "vec3.h"
#include "edge.h"
#include "sphere.h"
#include "mat4.h"
#include "bbox.h"

typedef struct
{
    Vec3 a;
    Vec3 b;
    Vec3 c;
} Triangle;

#define triangle(a, b, c) ((Triangle){a, b, c})

Sphere triangle_sqr_circumsphere(Triangle t);
bool triangle_circumfere(Triangle t, Vec3 p);
bool triangle_eq(Triangle a, Triangle b);
bool triangle_near_eq(Triangle a, Triangle b);
bool triangle_has_vertex(Triangle a, Vec3 p);
bool triangle_has_edge(Triangle a, Edge p);
Vec3 triangle_norm(Triangle t);
void triangle_edges(Triangle t, Edge edges[3]);
Triangle triangle_supra(Vec3 *vertices, int n, float offset);
Triangle triangle_offset(Triangle q, Vec3 b);
Triangle triangle_rot(Triangle t, Rot r);
Triangle calculate_supra(BBox b);
#endif