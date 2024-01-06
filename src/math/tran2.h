#ifndef cgame_TRAN2_H
#define cgame_TRAN2_H

#include "vec2.h"
#include "rot2.h"

typedef struct
{
    Vec2 position;
    Rot2 rotation;
} Tran2;

#define transform2(pos, rot) ((Tran2){pos, rot})

static const Tran2 tran2_zero = {vec2_zero, rot2_identity};

/// Transform a point (e.g. local space to world space)
static inline Vec2 tran2_transform(Tran2 xf, const Vec2 p)
{
	return vec2((xf.rotation.cos * p.x - xf.rotation.sin * p.y) + xf.position.x, (xf.rotation.sin * p.x + xf.rotation.cos * p.y) + xf.position.y);
}

/// Inverse transform a point (e.g. world space to local space)
static inline Vec2 tran2_untransform(Tran2 xf, const Vec2 p)
{
	float vx = p.x - xf.position.x;
	float vy = p.y - xf.position.y;
	return vec2(xf.rotation.cos * vx + xf.rotation.sin * vy, -xf.rotation.sin * vx + xf.rotation.cos * vy);
}

/// v2 = A.q.Rot(B.q.Rot(v1) + B.p) + A.p
///    = (A.q * B.q).Rot(v1) + A.q.Rot(B.p) + A.p
static inline Tran2 tran2_mul(Tran2 A, Tran2 B)
{
	Tran2 C;
	C.rotation = rot2_mul(A.rotation, B.rotation);
	C.position = vec2_add(rot2_rotate(A.rotation, B.position), A.position);
	return C;
}

/// v2 = A.q' * (B.q * v1 + B.p - A.p)
///    = A.q' * B.q * v1 + A.q' * (B.p - A.p)
static inline Tran2 tran2_unmul(Tran2 A, Tran2 B)
{
	Tran2 C;
	C.rotation = rot2_unmul(A.rotation, B.rotation);
	C.position = rot2_unrotate(A.rotation, vec2_sub(B.position, A.position));
	return C;
}


#endif