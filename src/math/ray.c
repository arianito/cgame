#include "ray.h"

#include <stdlib.h>

#include "scalar.h"

#include "caps2.h"

Ray ray_from_to(Vec3 a, Vec3 b)
{
    Ray r;
    r.origin = a;
    r.direction = vec3_sub(b, a);
    return r;
}

Ray ray_from_rot(Vec3 a, Rot b)
{
    Ray r;
    r.origin = a;
    r.direction = rot_forward(b);
    return r;
}

Ray ray_scale(Ray r, float scale)
{
    r.direction = vec3_mulf(r.direction, scale);
    return r;
}

Ray ray_move(Ray r, float distance)
{
    r.origin = vec3_add(r.origin, vec3_mulf(r.direction, distance));
    return r;
}

bool ray_hit_sphere(Ray r, Sphere s, Vec3 *hit)
{
    Vec3 oc = vec3_sub(s.position, r.origin);
    float a = vec3_sqr_length(r.direction);
    float b = 2.0f * (oc.x * r.direction.x +
                      oc.y * r.direction.y +
                      oc.z * r.direction.z);
    float c = vec3_sqr_length(oc) - squaref(s.radius);
    float dis = squaref(b) - (4 * a * c);
    if (dis < 0)
        return false;
    if (hit != NULL)
    {
        float t = -minf((-b - sqrf(dis)) / (2.0f * a),
                        (-b + sqrf(dis)) / (2.0f * a));
        *hit = vec3_add(r.origin, vec3_mulf(r.direction, t));
    }
    return true;
}

bool ray_hit_circle(Ray r, Sphere s, Vec3 normal, Vec3 *hit)
{
    Vec3 v = vec3_sub(s.position, r.origin);
    float de = vec3_dot(r.direction, normal);
    if (de < EPSILON)
        return false;

    float t = vec3_dot(v, normal) / de;
    Vec3 itp = vec3_add(r.origin, vec3_mulf(r.direction, t));
    float dist = vec3_length(vec3_sub(itp, s.position));

    if (hit != NULL)
        *hit = vec3_add(r.origin, vec3_mulf(r.direction, t));
    return (bool)(dist <= s.radius);
}

bool ray_hit_bbox(Ray r, BBox b, Vec3 *hit)
{
    Vec3 re;
    re.x = 1.0f / r.direction.x;
    re.y = 1.0f / r.direction.y;
    re.z = 1.0f / r.direction.z;

    float t1 = (b.min.x - r.origin.x) * re.x;
    float t2 = (b.max.x - r.origin.x) * re.x;

    float t3 = (b.min.y - r.origin.y) * re.y;
    float t4 = (b.max.y - r.origin.y) * re.y;

    float t5 = (b.min.z - r.origin.z) * re.z;
    float t6 = (b.max.z - r.origin.z) * re.z;

    float int_min = maxf(maxf(minf(t1, t2), minf(t3, t4)), minf(t5, t6));
    float int_max = minf(minf(maxf(t1, t2), maxf(t3, t4)), maxf(t5, t6));

    if (int_max < 0)
        return false;

    if (int_min > int_max)
        return false;

    if (hit != NULL)
        *hit = vec3_add(r.origin, vec3_mulf(r.direction, int_max));

    return true;
}

bool ray_hit_triangle(Ray r, Triangle tri, Vec3 *hit)
{

    Vec3 edge1 = vec3_sub(tri.b, tri.a);
    Vec3 edge2 = vec3_sub(tri.c, tri.a);

    Vec3 h = vec3_cross(r.direction, edge2);
    float a = vec3_dot(edge1, h);

    if (absf(a) < EPSILON)
        return false;

    float f = 1.0f / a;
    Vec3 s = vec3_sub(r.origin, tri.a);
    float u = f * vec3_dot(s, h);

    if (u < 0.0f || u > 1.0f)
        return false;

    Vec3 q = vec3_cross(s, edge1);
    float v = f * vec3_dot(r.direction, q);

    if (v < 0.0f || u + v > 1.0f)
        return false;

    float t = f * vec3_dot(edge2, q);

    if (t > EPSILON)
    {

        if (hit != NULL)
            *hit = vec3_add(r.origin, vec3_mulf(r.direction, t));
        return true;
    }

    return false;
}

bool ray_hit_quad(Ray r, Quad q, Vec3 *hit)
{
    if (ray_hit_triangle(r, (Triangle){q.a, q.b, q.c}, hit))
    {
        return true;
    }
    if (ray_hit_triangle(r, (Triangle){q.a, q.c, q.d}, hit))
    {
        return true;
    }
    return false;
}
