#ifndef cgame_ROT_H
#define cgame_ROT_H

#include <stdbool.h>

#include "rot_ifc.h"
#include "mat4_ifc.h"
#include "quat.h"
#include "vec3.h"

bool rot_eq(Rot a, Rot b);
bool rot_eq0(Rot a);
bool rot_near_eq(Rot a, Rot b);
bool rot_near0(Rot a);
Rot rot_add(Rot a, Rot b);
Rot rot_addf(Rot a, float b);
Rot rot_sub(Rot u, Rot v);
Rot rot_subf(Rot a, float b);
Rot rot_mulf(Rot a, float b);
Rot rot_neg(Rot a);
Rot rot_snap(Rot a, float size);
Rot rot_inv(Rot a);
Rot rot_clamp(Rot a);
bool rot_nan(Rot a);
Rot rot_norm(Rot a);
Rot rot_denorm(Rot a);
float rot_dist(Rot a, Rot b);
Rot rot_eqv(Rot a);
Rot rot_lerp(Rot a, Rot b, float dt);
Rot rot_lerp01(Rot a, Rot b, float dt);
Rot rot_rlerp(Rot a, Rot b, float dt);
Mat4 rot_matrix(Rot a, Vec3 origin);
Vec3 rot_forward(Rot a);
Quat rot_quat(Rot a);
Vec3 rot_right(Rot a);
Vec3 rot_up(Rot a);
Vec3 rot_rotate(Rot r, Vec3 b);
Vec3 rot_unrotate(Rot r, Vec3 b);
Rot rot_look_at(Vec3 a, Vec3 b);

#endif