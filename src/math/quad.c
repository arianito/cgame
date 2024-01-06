#include "quad.h"

Quad quad_rot(Quad q, Rot r)
{
    q.a = rot_rotate(r, q.a);
    q.b = rot_rotate(r, q.b);
    q.c = rot_rotate(r, q.c);
    q.d = rot_rotate(r, q.d);
    return q;
}
Quad quad_offset(Quad q, Vec3 b)
{
    q.a = vec3_add(b, q.a);
    q.b = vec3_add(b, q.b);
    q.c = vec3_add(b, q.c);
    q.d = vec3_add(b, q.d);
    return q;
}