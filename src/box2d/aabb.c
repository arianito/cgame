// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#include "aabb.h"
#include "box2d/constants.h"
#include "math/scalar.h"

bool b2AABB_IsValid(AABB a)
{
	Vec2 d = vec2_sub(a.max, a.min);
	bool valid = d.x >= 0.0f && d.y >= 0.0f;
	valid = valid && vec2_valid(a.min) && vec2_valid(a.max);
	return valid;
}

// From Real-time Collision Detection, p179.
b2RayCastOutput b2AABB_RayCast(AABB a, Vec2 p1, Vec2 p2)
{
	// Radius not handled
	b2RayCastOutput output = {0};

	float tmin = -MAX_FLOAT;
	float tmax = MAX_FLOAT;

	Vec2 p = p1;
	Vec2 d = vec2_sub(p2, p1);
	Vec2 absD = vec2_abs(d);

	Vec2 normal = vec2_zero;

	// x-coordinate
	if (absD.x < EPSILON)
	{
		// parallel
		if (p.x < a.min.x || a.max.x < p.x)
		{
			return output;
		}
	}
	else
	{
		float inv_d = 1.0f / d.x;
		float t1 = (a.min.x - p.x) * inv_d;
		float t2 = (a.max.x - p.x) * inv_d;

		// Sign of the normal vector.
		float s = -1.0f;

		if (t1 > t2)
		{
			float tmp = t1;
			t1 = t2;
			t2 = tmp;
			s = 1.0f;
		}

		// Push the min up
		if (t1 > tmin)
		{
			normal.y = 0.0f;
			normal.x = s;
			tmin = t1;
		}

		// Pull the max down
		tmax = minf(tmax, t2);

		if (tmin > tmax)
		{
			return output;
		}
	}

	// y-coordinate
	if (absD.y < EPSILON)
	{
		// parallel
		if (p.y < a.min.y || a.max.y < p.y)
		{
			return output;
		}
	}
	else
	{
		float inv_d = 1.0f / d.y;
		float t1 = (a.min.y - p.y) * inv_d;
		float t2 = (a.max.y - p.y) * inv_d;

		// Sign of the normal vector.
		float s = -1.0f;

		if (t1 > t2)
		{
			float tmp = t1;
			t1 = t2;
			t2 = tmp;
			s = 1.0f;
		}

		// Push the min up
		if (t1 > tmin)
		{
			normal.x = 0.0f;
			normal.y = s;
			tmin = t1;
		}

		// Pull the max down
		tmax = minf(tmax, t2);

		if (tmin > tmax)
		{
			return output;
		}
	}

	// Does the ray start inside the box?
	// Does the ray intersect beyond the max fraction?
	if (tmin < 0.0f || 1.0f < tmin)
	{
		return output;
	}

	// Intersection.
	output.fraction = tmin;
	output.normal = normal;
	output.point = vec2_lerp(p1, p2, tmin);
	output.hit = true;
	return output;
}