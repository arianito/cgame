#ifndef cgame_MAT4_H
#define cgame_MAT4_H

#include <stdint.h>
#include <stdbool.h>

#include "rot_ifc.h"
#include "mat4_ifc.h"
#include "vec3.h"
#include "vec4.h"
#include "defs.h"
#include "plane.h"

Mat4 mat4_transpose(Mat4 a);
Mat4 mat4_mul(Mat4 a, Mat4 b);
Mat4 mat4_mul3(Mat4 a, Mat4 b, Mat4 c);
Mat4 mat4_add(Mat4 a, Mat4 b);
Mat4 mat4_sub(Mat4 a, Mat4 b);
Mat4 mat4_mulf(Mat4 a, float b);
bool mat4_eq(Mat4 a, Mat4 b);
bool mat4_near_eq(Mat4 a, Mat4 b);
Vec4 mat4_mulv4(Mat4 a, Vec4 b);
Vec3 mat4_mulv3(Mat4 a, Vec3 b, float w);
Mat4 mat4_scale(Vec3 a);
Mat4 mat4_scalef(float a);
bool mat4_valid(Mat4 a);
Mat4 mat4_origin(Vec3 a);
Vec3 mat4_axis(Mat4 a, UnitAxisEnum ax);
void mat4_axes(Mat4 a, Vec3 *ax, Vec3 *ay, Vec3 *az);
Mat4 mat4_orthographic(float left, float right, float bottom, float top, float nr, float far);
Mat4 mat4_perspective(float fov, float aspect, float nr, float fr);
Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up);
Mat4 mat4_vec3(Vec3 xa, Vec3 ya, Vec3 za, Vec3 wa);
Mat4 mat4_plane(Plane a, Plane b, Plane c, Plane d);
Rot mat4_rot(Mat4 m);
Mat4 mat4_inverse_rot(Rot a);
Mat4 mat4_from_x(Vec3 x);
Mat4 mat4_from_y(Vec3 y);
Mat4 mat4_from_z(Vec3 z);
Mat4 mat4_view(Vec3 a, Rot b);
Mat4 mat4_transform(Vec3 origin, float scale);
float mat4_det(Mat4 m);
Mat4 mat4_inv(Mat4 m);

#endif