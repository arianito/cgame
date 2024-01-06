#include "edge.h"

Edge edge_offset(Edge q, Vec3 b)
{
    q.a = vec3_add(b, q.a);
    q.b = vec3_add(b, q.b);
    return q;
}
int edge_eq(Edge a, Edge b)
{
    return (
        (vec3_eq(a.a, b.a) && vec3_eq(a.b, b.b)) ||
        (vec3_eq(a.a, b.b) && vec3_eq(a.b, b.a)));
}

int edge_near_eq(Edge a, Edge b)
{
    return (
        (vec3_near_eq(a.a, b.a) && vec3_near_eq(a.b, b.b)) ||
        (vec3_near_eq(a.a, b.b) && vec3_near_eq(a.b, b.a)));
}

int edge_has_vertex(Edge a, Vec3 p)
{
    return (vec3_near_eq(a.a, p) || vec3_near_eq(a.b, p));
}

Vec3 edge_reflect(Vec3 ea, Vec3 eb, Vec3 point)
{
    float a = eb.y - ea.y;
    float b = ea.x - eb.x;
    float c = eb.x * ea.y - ea.x * eb.y;

    float footX = (b * (b * point.x - a * point.y) - a * c) / (a * a + b * b);
    float footY = (a * (a * point.y - b * point.x) - b * c) / (a * a + b * b);

    Vec3 out;
    out.z = 0;
    out.x = 2 * footX - point.x;
    out.y = 2 * footY - point.y;
    return out;
}