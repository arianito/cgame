#ifndef cgame_RAY_H
#define cgame_RAY_H

#include <stdbool.h>
#include <stdint.h>

#include "vec3.h"
#include "rot.h"
#include "quad.h"
#include "sphere.h"
#include "bbox.h"
#include "quad.h"
#include "triangle.h"
#include "defs.h"

typedef struct
{
    Vec3 origin;
    Vec3 direction;
} Ray;

#define ray(o, d) ((Ray){o, d})

Ray ray_from_to(Vec3 a, Vec3 b);
Ray ray_from_rot(Vec3 a, Rot b);
Ray ray_scale(Ray r, float scale);
Ray ray_move(Ray r, float distance);
bool ray_hit_sphere(Ray r, Sphere s, Vec3 *hit);
bool ray_hit_circle(Ray r, Sphere s, Vec3 normal, Vec3 *hit);
bool ray_hit_bbox(Ray r, BBox b, Vec3 *hit);
bool ray_hit_triangle(Ray r, Triangle tri, Vec3 *hit);
bool ray_hit_quad(Ray r, Quad q, Vec3 *hit);

#endif