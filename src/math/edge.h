#ifndef cgame_EDGE_H
#define cgame_EDGE_H

#include "vec3.h"

typedef struct
{
    Vec3 a;
    Vec3 b;
} Edge;

#define edge(a, b) ((Edge){a, b})

int edge_eq(Edge a, Edge b);
int edge_near_eq(Edge a, Edge b);
int edge_has_vertex(Edge a, Vec3 p);
Vec3 edge_reflect(Vec3 ea, Vec3 eb, Vec3 point);
Edge edge_offset(Edge q, Vec3 b);
#endif