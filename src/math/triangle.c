#include "triangle.h"

#include "rot.h"
#include "scalar.h"

Sphere triangle_sqr_circumsphere(Triangle t)
{
    Sphere s;

    float ap = mat4_det((Mat4){{{1, 0, 0, 0},
                                {0, t.a.x, t.b.x, t.c.x},
                                {0, t.a.y, t.b.y, t.c.y},
                                {0, 1, 1, 1}}});
    float a2 = vec3_sqr_length(t.a);
    float b2 = vec3_sqr_length(t.b);
    float c2 = vec3_sqr_length(t.c);

    float bx = -mat4_det((Mat4){{{1, 0, 0, 0},
                                 {0, a2, b2, c2},
                                 {0, t.a.y, t.b.y, t.c.y},
                                 {0, 1, 1, 1}}});

    float by = mat4_det((Mat4){{{1, 0, 0, 0},
                                {0, a2, b2, c2},
                                {0, t.a.x, t.b.x, t.c.x},
                                {0, 1, 1, 1}}});

    float cp = -mat4_det((Mat4){{{1, 0, 0, 0},
                                 {0, a2, b2, c2},
                                 {0, t.a.x, t.b.x, t.c.x},
                                 {0, t.a.y, t.b.y, t.c.y}}});

    s.position = vec3(-bx / (2 * ap), -by / (2 * ap), 0);
    s.radius = ((bx * bx) + (by * by) - (4 * (ap * cp))) / (4 * (ap * ap));
    return s;
}

bool triangle_circumfere(Triangle t, Vec3 p)
{
    Sphere s = triangle_sqr_circumsphere(t);
    return vec3_sqr_length(vec3_sub(s.position, p)) <= s.radius;
}

bool triangle_eq(Triangle a, Triangle b)
{
    return (vec3_eq(a.a, b.a) && vec3_eq(a.b, b.b) && vec3_eq(a.c, b.c)) ||
           (vec3_eq(a.a, b.b) && vec3_eq(a.b, b.c) && vec3_eq(a.c, b.a)) ||
           (vec3_eq(a.a, b.c) && vec3_eq(a.b, b.a) && vec3_eq(a.c, b.b));
}

bool triangle_near_eq(Triangle a, Triangle b)
{
    return (vec3_near_eq(a.a, b.a) && vec3_near_eq(a.b, b.b) && vec3_near_eq(a.c, b.c)) ||
           (vec3_near_eq(a.a, b.b) && vec3_near_eq(a.b, b.c) && vec3_near_eq(a.c, b.a)) ||
           (vec3_near_eq(a.a, b.c) && vec3_near_eq(a.b, b.a) && vec3_near_eq(a.c, b.b));
}

bool triangle_has_vertex(Triangle a, Vec3 p)
{
    return (vec3_near_eq(a.a, p) || vec3_near_eq(a.b, p) || vec3_near_eq(a.c, p));
}

bool triangle_has_edge(Triangle a, Edge p)
{
    return (vec3_near_eq(a.a, p.a) && vec3_near_eq(a.b, p.b)) ||
           (vec3_near_eq(a.a, p.b) && vec3_near_eq(a.b, p.a)) ||
           (vec3_near_eq(a.b, p.a) && vec3_near_eq(a.c, p.b)) ||
           (vec3_near_eq(a.b, p.b) && vec3_near_eq(a.c, p.a)) ||
           (vec3_near_eq(a.a, p.a) && vec3_near_eq(a.c, p.b)) ||
           (vec3_near_eq(a.a, p.b) && vec3_near_eq(a.c, p.a));
}

Triangle triangle_supra(Vec3 *vertices, int n, float offset)
{
    BBox b = bbox_calculate(vertices, n);
    b = bbox_expand(b, offset);
    float max = maxf(bbox_width(b), bbox_depth(b)) * 2;
    Triangle t;
    t.a = b.min;
    t.b = b.min;
    t.c = b.min;
    t.b.y += max;
    t.c.x += max;
    return t;
}

Vec3 triangle_norm(Triangle t)
{
    return vec3_cross(vec3_sub(t.b, t.a), vec3_sub(t.c, t.a));
}

void triangle_edges(Triangle t, Edge edges[3])
{
    edges[0] = (Edge){t.a, t.b};
    edges[1] = (Edge){t.b, t.c};
    edges[2] = (Edge){t.c, t.a};
}

Triangle triangle_offset(Triangle q, Vec3 b)
{
    q.a = vec3_add(b, q.a);
    q.b = vec3_add(b, q.b);
    q.c = vec3_add(b, q.c);
    return q;
}

Triangle triangle_rot(Triangle t, Rot r)
{
    t.a = rot_rotate(r, t.a);
    t.b = rot_rotate(r, t.b);
    t.c = rot_rotate(r, t.c);
    return t;
}
Triangle calculate_supra(BBox b)
{
    float max = maxf(bbox_width(b), bbox_depth(b)) * 2;
    return (Triangle){b.min, vec3_add(b.min, vec3_mulf(vec3_right, max)), vec3_add(b.min, vec3_mulf(vec3_forward, max))};
}
