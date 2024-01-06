#ifndef cgame_VEC3_H
#define cgame_VEC3_H

#include <stdbool.h>
#include "vec3_ifc.h"
#include "vec2_ifc.h"

inline static Vec3 vec3yz(Vec2 a) { return vec3(0, a.x, a.y); }
inline static Vec3 vec3xy(Vec2 a) { return vec3(a.x, a.y, 0); }
inline static Vec3 vec3xz(Vec2 a) { return vec3(a.x, 0, a.y); }

Vec3 vec3_rotate_xy(Vec3 a, float d);
Vec3 vec3_randv(Vec3 halfBound);
Vec3 vec3_randf(float x, float y, float z);
Vec3 vec3_neg(Vec3 a);
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_center(Vec3 a, Vec3 b);
Vec3 vec3_addf(Vec3 a, float b);
Vec3 vec3_sub(Vec3 u, Vec3 v);
Vec3 vec3_subf(Vec3 a, float b);
Vec3 vec3_mulf(Vec3 a, float b);
float vec3_dot(Vec3 a, Vec3 b);
float vec3_sqr_length(Vec3 a);
float vec3_length(Vec3 a);
float vec3_dist(Vec3 a, Vec3 b);
float vec3_dot_xy(Vec3 a, Vec3 b);
float vec3_sqr_length_xy(Vec3 a);
float vec3_length_xy(Vec3 a);
float vec3_distance_xy(Vec3 a, Vec3 b);
bool vec3_eq(Vec3 a, Vec3 b);
bool vec3_eq0(Vec3 a);
bool vec3_near_eq(Vec3 a, Vec3 b);
bool vec3_near0(Vec3 a);
Vec3 vec3_cross(Vec3 a, Vec3 b);
Vec3 vec3_min(Vec3 a, Vec3 b);
float vec3_min_comp(Vec3 a);
Vec3 vec3_max(Vec3 a, Vec3 b);
float vec3_max_comp(Vec3 a);
Vec3 vec3_abs(Vec3 a);
float vec3_abs_min(Vec3 a);
float vec3_abs_max(Vec3 a);
Vec3 vec3_norm(Vec3 a);
Vec3 vec3_norm_xy(Vec3 a);
Vec3 vec3_sign(Vec3 a);
Vec3 vec3_lerp(Vec3 a, Vec3 b, float dt);
Vec3 vec3_lerp01(Vec3 a, Vec3 b, float dt);
Vec3 vec3_move_towards(Vec3 current, Vec3 target, float maxDelta);
Vec3 vec3_reflect(Vec3 a, Vec3 n);
Vec3 vec3_projection(Vec3 a);
Vec3 vec3_project(Vec3 a, Vec3 b);
Vec3 vec3_project_normal(Vec3 a, Vec3 n);
Vec3 vec3_mirror(Vec3 a, Vec3 n);
Vec3 vec3_snap(Vec3 a, float size);
Vec3 vec3_snap_cube(Vec3 a, float size);
Vec3 vec3_snap_cube_xy(Vec3 a, float size);
Vec3 vec3_clamp(Vec3 a, Vec3 min, Vec3 max);
Vec3 vec3_clampf(Vec3 a, float min, float max);
Vec3 vec3_clampf_xy(Vec3 a, float min, float max);
Vec3 vec3_reciprocal(Vec3 a);
float vec3_angle(Vec3 a, Vec3 b);
float vec3_signed_angle(Vec3 a, Vec3 b, Vec3 axis);
float vec3_cos_angle_xy(Vec3 a, Vec3 b);
Vec3 vec3_intersect_plane(Vec3 start, Vec3 end, Vec3 planeOrigin, Vec3 planeNormal);

#endif