#ifndef cgame_DRAW_H
#define cgame_DRAW_H

#include "math/vec2.h"
#include "math/vec3.h"
#include "math/color.h"
#include "math/bbox.h"
#include "math/quad.h"
#include "math/ray.h"
#include "math/triangle.h"
#include "math/tetrahedron.h"
#include "math/aabb.h"

void draw_init();

void draw_render();

void draw_terminate();

void draw_point(Vec3 pos, float size, Color c);

void draw_line(Vec3 a, Vec3 b, Color c);

void draw_normal(Vec3 a, Vec3 n, float scale, Color c);

void draw_bbox(BBox bbox, Color c);

void fill_bbox(BBox bbox, Color c, bool cull);

void draw_quad(Quad q, Color cl);

void fill_quad(Quad q, Color cl, bool cull);

void draw_aabb_yz(AABB a, Color cl);

void fill_aabb_yz(AABB a, Color cl, bool cull);

void draw_cube(Vec3 a, Vec3 s, Color c);

void fill_cube(Vec3 a, Vec3 s, Color c, bool cull);

void draw_cubef(Vec3 a, float s, Color c);

void fill_cubef(Vec3 a, float s, Color c, bool cull);

void draw_edge(Edge e, Color c);

void draw_triangle(Triangle t, Color c);

void fill_triangle(Triangle t, Color c, bool cull);

void draw_tetrahedron(Tetrahedron t, Color c);

void fill_tetrahedron(Tetrahedron t, Color c, bool cull);

void draw_circle_xy(Vec3 center, float radius, Color color, int seg);

void fill_circle_xy(Vec3 center, float radius, Color color, int seg, bool cull);

void draw_circle_xz(Vec3 center, float radius, Color color, int seg);

void draw_circle_yz(Vec3 center, float radius, Color color, int seg);

void fill_circle_yz(Vec3 center, float radius, Color color, int seg, bool cull);

void draw_capsule_yz(Vec2 p1, Vec2 p2, float radius, Color color, int seg);

void fill_capsule_yz(Vec2 p1, Vec2 p2, float radius, Color color, int seg, bool cull);

void draw_sphere(Vec3 a, float r, Color c, int s);

void draw_arrow(Vec3 a, Vec3 b, Vec3 up, Color c, float p);

void draw_ray(Ray r, Color c);

void draw_axis(Vec3 a, float scale, Quat q);

void draw_axisRot(Vec3 a, float scale, Rot r);

void draw_frustum(Vec3 pos, Rot rt, float fov, float ratio, float nr, float fr, float ofc, Color c);

void draw_polygon(const Vec3 vertices[], int n, Color color);

void fill_polygon(const Vec3 vertices[], int n, Color color, bool cull);

void draw_polygon_yz(const Vec2 vertices[], int n, Color color);
void fill_polygon_yz(const Vec2 vertices[], int n, Color color, bool cull);

void draw_polygon_xy(const Vec2 vertices[], int n, Color color);
void fill_polygon_xy(const Vec2 vertices[], int n, Color color, bool cull);


#endif