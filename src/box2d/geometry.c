// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#include "box2d/geometry.h"

#include "aabb.h"
#include "core.h"
#include "shape.h"

#include "box2d/distance.h"
#include "box2d/hull.h"

#include <float.h>

bool b2IsValidRay(const b2RayCastInput* input)
{
	bool isValid = vec2_valid(input->origin) && vec2_valid(input->translation) && validf(input->maxFraction) &&
				   0.0f <= input->maxFraction && input->maxFraction < b2_huge;
	return isValid;
}

static Vec2 b2ComputePolygonCentroid(const Vec2* vertices, int32_t count)
{
	Vec2 center = {0.0f, 0.0f};
	float area = 0.0f;

	// Get a reference point for forming triangles.
	// Use the first vertex to reduce round-off errors.
	Vec2 origin = vertices[0];

	const float inv3 = 1.0f / 3.0f;

	for (int32_t i = 1; i < count - 1; ++i)
	{
		// Triangle edges
		Vec2 e1 = vec2_sub(vertices[i], origin);
		Vec2 e2 = vec2_sub(vertices[i + 1], origin);
		float a = 0.5f * vec2_cross(e1, e2);

		// Area weighted centroid
		center = vec2_mul_add(center, a * inv3, vec2_add(e1, e2));
		area += a;
	}

	
	float invArea = 1.0f / area;
	center.x *= invArea;
	center.y *= invArea;

	// Restore offset
	center = vec2_add(origin, center);

	return center;
}

b2Polygon b2MakePolygon(const b2Hull* hull, float radius)
{
	

	b2Polygon shape = {0};
	shape.count = hull->count;
	shape.radius = radius;

	// Copy vertices
	for (int32_t i = 0; i < shape.count; ++i)
	{
		shape.vertices[i] = hull->points[i];
	}

	// Compute normals. Ensure the edges have non-zero length.
	for (int32_t i = 0; i < shape.count; ++i)
	{
		int32_t i1 = i;
		int32_t i2 = i + 1 < shape.count ? i + 1 : 0;
		Vec2 edge = vec2_sub(shape.vertices[i2], shape.vertices[i1]);
		
		shape.normals[i] = vec2_norm(vec2_crossvf(edge, 1.0f));
	}

	shape.centroid = b2ComputePolygonCentroid(shape.vertices, shape.count);

	return shape;
}

b2Polygon b2MakeOffsetPolygon(const b2Hull* hull, float radius, Tran2 transform)
{
	

	b2Polygon shape = {0};
	shape.count = hull->count;
	shape.radius = radius;

	// Copy vertices
	for (int32_t i = 0; i < shape.count; ++i)
	{
		shape.vertices[i] = tran2_transform(transform, hull->points[i]);
	}

	// Compute normals. Ensure the edges have non-zero length.
	for (int32_t i = 0; i < shape.count; ++i)
	{
		int32_t i1 = i;
		int32_t i2 = i + 1 < shape.count ? i + 1 : 0;
		Vec2 edge = vec2_sub(shape.vertices[i2], shape.vertices[i1]);
		
		shape.normals[i] = vec2_norm(vec2_crossvf(edge, 1.0f));
	}

	shape.centroid = b2ComputePolygonCentroid(shape.vertices, shape.count);

	return shape;
}

b2Polygon b2MakeSquare(float h)
{
	return b2MakeBox(h, h);
}

b2Polygon b2MakeBox(float hx, float hy)
{
	
	

	b2Polygon shape = {0};
	shape.count = 4;
	shape.vertices[0] = vec2(-hx, -hy);
	shape.vertices[1] = vec2(hx, -hy);
	shape.vertices[2] = vec2(hx, hy);
	shape.vertices[3] = vec2(-hx, hy);
	shape.normals[0] = vec2(0.0f, -1.0f);
	shape.normals[1] = vec2(1.0f, 0.0f);
	shape.normals[2] = vec2(0.0f, 1.0f);
	shape.normals[3] = vec2(-1.0f, 0.0f);
	shape.radius = 0.0f;
	shape.centroid = vec2_zero;
	return shape;
}

b2Polygon b2MakeRoundedBox(float hx, float hy, float radius)
{
	b2Polygon shape = b2MakeBox(hx, hy);
	shape.radius = radius;
	return shape;
}

b2Polygon b2MakeOffsetBox(float hx, float hy, Vec2 center, float angle)
{
	Tran2 xf;
	xf.position = center;
	xf.rotation = rot2f(angle);

	b2Polygon shape = {0};
	shape.count = 4;
	shape.vertices[0] = tran2_transform(xf, vec2(-hx, -hy));
	shape.vertices[1] = tran2_transform(xf, vec2(hx, -hy));
	shape.vertices[2] = tran2_transform(xf, vec2(hx, hy));
	shape.vertices[3] = tran2_transform(xf, vec2(-hx, hy));
	shape.normals[0] = rot2_rotate(xf.rotation, vec2(0.0f, -1.0f));
	shape.normals[1] = rot2_rotate(xf.rotation, vec2(1.0f, 0.0f));
	shape.normals[2] = rot2_rotate(xf.rotation, vec2(0.0f, 1.0f));
	shape.normals[3] = rot2_rotate(xf.rotation, vec2(-1.0f, 0.0f));
	shape.radius = 0.0f;
	shape.centroid = center;
	return shape;
}

b2Polygon b2TransformPolygon(Tran2 transform, const b2Polygon* polygon)
{
	b2Polygon p = *polygon;

	for (int i = 0; i < p.count; ++i)
	{
		p.vertices[i] = tran2_transform(transform, p.vertices[i]);
		p.normals[i] = rot2_rotate(transform.rotation, p.normals[i]);
	}

	p.centroid = tran2_transform(transform, p.centroid);

	return p;
}

b2MassData b2ComputeCircleMass(const b2Circle* shape, float density)
{
	float rr = shape->radius * shape->radius;

	b2MassData massData;
	massData.mass = density * b2_pi * rr;
	massData.center = shape->point;

	// inertia about the local origin
	massData.I = massData.mass * (0.5f * rr + vec2_dot(shape->point, shape->point));

	return massData;
}

b2MassData b2ComputeCapsuleMass(const b2Capsule* shape, float density)
{
	float radius = shape->radius;
	float rr = radius * radius;
	Vec2 p1 = shape->point1;
	Vec2 p2 = shape->point2;
	float length = vec2_length(vec2_sub(p2, p1));
	float ll = length * length;

	float circleMass = density * (b2_pi * radius * radius);
	float boxMass = density * (2.0f * radius * length);

	b2MassData massData;
	massData.mass = circleMass + boxMass;
	massData.center.x = 0.5f * (p1.x + p2.x);
	massData.center.y = 0.5f * (p1.y + p2.y);

	// two offset half circles, both halves add up to full circle and each half is offset by half length
	// semi-circle centroid = 4 r / 3 pi
	// Need to apply parallel-axis theorem twice:
	// 1. shift semi-circle centroid to origin
	// 2. shift semi-circle to box end
	// m * ((h + lc)^2 - lc^2) = m * (h^2 + 2 * h * lc)
	// See: https://en.wikipedia.org/wiki/Parallel_axis_theorem
	// I verified this formula by computing the convex hull of a 128 vertex capsule

	// half circle centroid
	float lc = 4.0f * radius / (3.0f * b2_pi);

	// half length of rectangular portion of capsule
	float h = 0.5f * length;

	float circleInertia = circleMass * (0.5f * rr + h * h + 2.0f * h * lc);
	float boxInertia = boxMass * (4.0f * rr + ll) / 12.0f;
	massData.I = circleInertia + boxInertia;

	return massData;
}

b2MassData b2ComputePolygonMass(const b2Polygon* shape, float density)
{
	// Polygon mass, centroid, and inertia.
	// Let rho be the polygon density in mass per unit area.
	// Then:
	// mass = rho * int(dA)
	// centroid.x = (1/mass) * rho * int(x * dA)
	// centroid.y = (1/mass) * rho * int(y * dA)
	// I = rho * int((x*x + y*y) * dA)
	//
	// We can compute these integrals by summing all the integrals
	// for each triangle of the polygon. To evaluate the integral
	// for a single triangle, we make a change of variables to
	// the (u,v) coordinates of the triangle:
	// x = x0 + e1x * u + e2x * v
	// y = y0 + e1y * u + e2y * v
	// where 0 <= u && 0 <= v && u + v <= 1.
	//
	// We integrate u from [0,1-v] and then v from [0,1].
	// We also need to use the Jacobian of the transformation:
	// D = cross(e1, e2)
	//
	// Simplification: triangle centroid = (1/3) * (p1 + p2 + p3)
	//
	// The rest of the derivation is handled by computer algebra.

	

	if (shape->count == 1)
	{
		b2Circle circle;
		circle.point = shape->vertices[0];
		circle.radius = shape->radius;
		return b2ComputeCircleMass(&circle, density);
	}

	if (shape->count == 2)
	{
		b2Capsule capsule;
		capsule.point1 = shape->vertices[0];
		capsule.point2 = shape->vertices[1];
		capsule.radius = shape->radius;
		return b2ComputeCapsuleMass(&capsule, density);
	}

	Vec2 vertices[b2_maxPolygonVertices] = {0};
	int32_t count = shape->count;
	float radius = shape->radius;

	if (radius > 0.0f)
	{
		// Push out vertices according to radius. This improves
		// the mass accuracy, especially the rotational inertia.
		for (int32_t i = 0; i < count; ++i)
		{
			int32_t j = i == 0 ? count - 1 : i - 1;
			Vec2 n1 = shape->normals[j];
			Vec2 n2 = shape->normals[i];

			Vec2 mid = vec2_norm(vec2_add(n1, n2));
			Vec2 t1 = {-n1.y, n1.x};
			float sinHalfAngle = vec2_cross(mid, t1);

			float offset = radius;
			if (sinHalfAngle > FLT_EPSILON)
			{
				offset = radius / sinHalfAngle;
			}

			vertices[i] = vec2_mul_add(shape->vertices[i], offset, mid);
		}
	}
	else
	{
		for (int32_t i = 0; i < count; ++i)
		{
			vertices[i] = shape->vertices[i];
		}
	}

	Vec2 center = {0.0f, 0.0f};
	float area = 0.0f;
	float I = 0.0f;

	// Get a reference point for forming triangles.
	// Use the first vertex to reduce round-off errors.
	Vec2 r = vertices[0];

	const float inv3 = 1.0f / 3.0f;

	for (int32_t i = 1; i < count - 1; ++i)
	{
		// Triangle edges
		Vec2 e1 = vec2_sub(vertices[i], r);
		Vec2 e2 = vec2_sub(vertices[i + 1], r);

		float D = vec2_cross(e1, e2);

		float triangleArea = 0.5f * D;
		area += triangleArea;

		// Area weighted centroid, r at origin
		center = vec2_mul_add(center, triangleArea * inv3, vec2_add(e1, e2));

		float ex1 = e1.x, ey1 = e1.y;
		float ex2 = e2.x, ey2 = e2.y;

		float intx2 = ex1 * ex1 + ex2 * ex1 + ex2 * ex2;
		float inty2 = ey1 * ey1 + ey2 * ey1 + ey2 * ey2;

		I += (0.25f * inv3 * D) * (intx2 + inty2);
	}

	b2MassData massData;

	// Total mass
	massData.mass = density * area;

	// Center of mass, shift back from origin at r
	
	float invArea = 1.0f / area;
	center.x *= invArea;
	center.y *= invArea;
	massData.center = vec2_add(r, center);

	// Inertia tensor relative to the local origin (point s).
	massData.I = density * I;

	// Shift to center of mass then to original body origin.
	massData.I += massData.mass * (vec2_dot(massData.center, massData.center) - vec2_dot(center, center));

	return massData;
}

AABB b2ComputeCircleAABB(const b2Circle* shape, Tran2 xf)
{
	Vec2 p = tran2_transform(xf, shape->point);
	float r = shape->radius;

	AABB aabb = {{p.x - r, p.y - r}, {p.x + r, p.y + r}};
	return aabb;
}

AABB b2ComputeCapsuleAABB(const b2Capsule* shape, Tran2 xf)
{
	Vec2 v1 = tran2_transform(xf, shape->point1);
	Vec2 v2 = tran2_transform(xf, shape->point2);

	Vec2 r = {shape->radius, shape->radius};
	Vec2 lower = vec2_sub(vec2_min(v1, v2), r);
	Vec2 upper = vec2_add(vec2_max(v1, v2), r);

	AABB aabb = {lower, upper};
	return aabb;
}

AABB b2ComputePolygonAABB(const b2Polygon* shape, Tran2 xf)
{
	
	Vec2 lower = tran2_transform(xf, shape->vertices[0]);
	Vec2 upper = lower;

	for (int32_t i = 1; i < shape->count; ++i)
	{
		Vec2 v = tran2_transform(xf, shape->vertices[i]);
		lower = vec2_min(lower, v);
		upper = vec2_max(upper, v);
	}

	Vec2 r = {shape->radius, shape->radius};
	lower = vec2_sub(lower, r);
	upper = vec2_add(upper, r);

	AABB aabb = {lower, upper};
	return aabb;
}

AABB b2ComputeSegmentAABB(const b2Segment* shape, Tran2 xf)
{
	Vec2 v1 = tran2_transform(xf, shape->point1);
	Vec2 v2 = tran2_transform(xf, shape->point2);

	Vec2 lower = vec2_min(v1, v2);
	Vec2 upper = vec2_max(v1, v2);

	AABB aabb = {lower, upper};
	return aabb;
}

bool b2PointInCircle(Vec2 point, const b2Circle* shape)
{
	Vec2 center = shape->point;
	return vec2_sqr_distance(point, center) <= shape->radius * shape->radius;
}

bool b2PointInCapsule(Vec2 point, const b2Capsule* shape)
{
	float rr = shape->radius * shape->radius;
	Vec2 p1 = shape->point1;
	Vec2 p2 = shape->point2;

	Vec2 d = vec2_sub(p2, p1);
	float dd = vec2_dot(d, d);
	if (dd == 0.0f)
	{
		// Capsule is really a circle
		return vec2_sqr_distance(point, p1) <= rr;
	}

	// Get closest point on capsule segment
	// c = p1 + t * d
	// dot(point - c, d) = 0
	// dot(point - p1 - t * d, d) = 0
	// t = dot(point - p1, d) / dot(d, d)
	float t = vec2_dot(vec2_sub(point, p1), d) / dd;
	t = clampf(t, 0.0f, 1.0f);
	Vec2 c = vec2_mul_add(p1, t, d);

	// Is query point within radius around closest point?
	return vec2_sqr_distance(point, c) <= rr;
}

bool b2PointInPolygon(Vec2 point, const b2Polygon* shape)
{
	b2DistanceInput input = {0};
	input.proxyA = b2MakeProxy(shape->vertices, shape->count, 0.0f);
	input.proxyB = b2MakeProxy(&point, 1, 0.0f);
	input.transformA = tran2_identity;
	input.transformB = tran2_identity;
	input.useRadii = false;

	b2DistanceCache cache = {0};
	b2DistanceOutput output = b2ShapeDistance(&cache, &input);

	return output.distance <= shape->radius;
}

// Precision Improvements for Ray / Sphere Intersection - Ray Tracing Gems 2019
// http://www.codercorner.com/blog/?p=321
b2RayCastOutput b2RayCastCircle(const b2RayCastInput* input, const b2Circle* shape)
{
	

	Vec2 p = shape->point;

	b2RayCastOutput output = {0};

	// Shift ray so circle center is the origin
	Vec2 s = vec2_sub(input->origin, p);
	float length;
	Vec2 d = vec2_length_normal(&length, input->translation);
	if (length == 0.0f)
	{
		// zero length ray
		return output;
	}

	// Find closest point on ray to origin

	// solve: dot(s + t * d, d) = 0
	float t = -vec2_dot(s, d);

	// c is the closest point on the line to the origin
	Vec2 c = vec2_mul_add(s, t, d);

	float cc = vec2_dot(c, c);
	float r = shape->radius;
	float rr = r * r;

	if (cc > rr)
	{
		// closest point is outside the circle
		return output;
	}

	// Pythagorus
	float h = sqrf(rr - cc);

	float fraction = t - h;

	if (fraction < 0.0f || input->maxFraction * length < fraction)
	{
		// outside the range of the ray segment
		return output;
	}

	Vec2 hitPoint = vec2_mul_add(s, fraction, d);

	output.fraction = fraction / length;
	output.normal = vec2_norm(hitPoint);
	output.point = vec2_mul_add(p, shape->radius, output.normal);
	output.hit = true;

	return output;
}

b2RayCastOutput b2RayCastCapsule(const b2RayCastInput* input, const b2Capsule* shape)
{
	

	b2RayCastOutput output = {0};

	Vec2 v1 = shape->point1;
	Vec2 v2 = shape->point2;

	Vec2 e = vec2_sub(v2, v1);

	float capsuleLength;
	Vec2 a = vec2_length_normal(&capsuleLength, e);

	if (capsuleLength < FLT_EPSILON)
	{
		// Capsule is really a circle
		b2Circle circle = {v1, shape->radius};
		return b2RayCastCircle(input, &circle);
	}

	Vec2 p1 = input->origin;
	Vec2 d = input->translation;

	// Ray from capsule start to ray start
	Vec2 q = vec2_sub(p1, v1);
	float qa = vec2_dot(q, a);

	// Vector to ray start that is perpendicular to capsule axis
	Vec2 qp = vec2_mul_add(q, -qa, a);

	float radius = shape->radius;

	// Does the ray start within the infinite length capsule?
	if (vec2_dot(qp, qp) < radius * radius)
	{
		if (qa < 0.0f)
		{
			// start point behind capsule segment
			b2Circle circle = {v1, shape->radius};
			return b2RayCastCircle(input, &circle);
		}

		if (qa > 1.0f)
		{
			// start point ahead of capsule segment
			b2Circle circle = {v2, shape->radius};
			return b2RayCastCircle(input, &circle);
		}

		// ray starts inside capsule -> no hit
		return output;
	}

	// Perpendicular to capsule axis, pointing right
	Vec2 n = {a.y, -a.x};

	float rayLength;
	Vec2 u = vec2_length_normal(&rayLength, d);

	// Intersect ray with infinite length capsule
	// v1 + radius * n + s1 * a = p1 + s2 * u
	// v1 - radius * n + s1 * a = p1 + s2 * u

	// s1 * a - s2 * u = b
	// b = q - radius * ap
	// or
	// b = q + radius * ap

	// Cramer's rule [a -u]
	float den = -a.x * u.y + u.x * a.y;
	if (-FLT_EPSILON < den && den < FLT_EPSILON)
	{
		// Ray is parallel to capsule and outside infinite length capsule
		return output;
	}

	Vec2 b1 = vec2_mul_sub(q, radius, n);
	Vec2 b2 = vec2_mul_add(q, radius, n);

	float invDen = 1.0f / den;

	// Cramer's rule [a b1]
	float s21 = (a.x * b1.y - b1.x * a.y) * invDen;

	// Cramer's rule [a b2]
	float s22 = (a.x * b2.y - b2.x * a.y) * invDen;

	float s2;
	Vec2 b;
	if (s21 < s22)
	{
		s2 = s21;
		b = b1;
	}
	else
	{
		s2 = s22;
		b = b2;
		n = vec2_neg(n);
	}

	if (s2 < 0.0f || input->maxFraction * rayLength < s2)
	{
		return output;
	}

	// Cramer's rule [b -u]
	float s1 = (-b.x * u.y + u.x * b.y) * invDen;

	if (s1 < 0.0f)
	{
		// ray passes behind capsule segment
		b2Circle circle = {v1, shape->radius};
		return b2RayCastCircle(input, &circle);
	}
	else if (capsuleLength < s1)
	{
		// ray passes ahead of capsule segment
		b2Circle circle = {v2, shape->radius};
		return b2RayCastCircle(input, &circle);
	}
	else
	{
		// ray hits capsule side
		output.fraction = s2 / rayLength;
		output.point = vec2_add(vec2_lerp(v1, v2, s1 / capsuleLength), vec2_mulfv(shape->radius, n));
		output.normal = n;
		output.hit = true;
		return output;
	}
}

// Ray vs line segment
b2RayCastOutput b2RayCastSegment(const b2RayCastInput* input, const b2Segment* shape, bool oneSided)
{
	if (oneSided)
	{
		// Skip left-side collision
		float offset = vec2_cross(vec2_sub(input->origin, shape->point1), vec2_sub(shape->point2, shape->point1));
		if (offset < 0.0f)
		{
			b2RayCastOutput output = {0};
			return output;
		}
	}

	// Put the ray into the edge's frame of reference.
	Vec2 p1 = input->origin;
	Vec2 d = input->translation;

	Vec2 v1 = shape->point1;
	Vec2 v2 = shape->point2;
	Vec2 e = vec2_sub(v2, v1);

	b2RayCastOutput output = {0};

	float length;
	Vec2 eUnit = vec2_length_normal(&length, e);
	if (length == 0.0f)
	{
		return output;
	}

	// Normal points to the right, looking from v1 towards v2
	Vec2 normal = vec2_perp_right(eUnit);

	// Intersect ray with infinite segment using normal
	// Similar to intersecting a ray with an infinite plane
	// p = p1 + t * d
	// dot(normal, p - v1) = 0
	// dot(normal, p1 - v1) + t * dot(normal, d) = 0
	float numerator = vec2_dot(normal, vec2_sub(v1, p1));
	float denominator = vec2_dot(normal, d);

	if (denominator == 0.0f)
	{
		// parallel
		return output;
	}

	float t = numerator / denominator;
	if (t < 0.0f || input->maxFraction < t)
	{
		// out of ray range
		return output;
	}

	// Intersection point on infinite segment
	Vec2 p = vec2_mul_add(p1, t, d);

	// Compute position of p along segment
	// p = v1 + s * e
	// s = dot(p - v1, e) / dot(e, e)

	float s = vec2_dot(vec2_sub(p, v1), eUnit);
	if (s < 0.0f || length < s)
	{
		// out of segment range
		return output;
	}

	if (numerator > 0.0f)
	{
		normal = vec2_neg(normal);
	}

	output.fraction = t;
	output.point = vec2_mul_add(p1, t, d);
	output.normal = normal;
	output.hit = true;

	return output;
}

b2RayCastOutput b2RayCastPolygon(const b2RayCastInput* input, const b2Polygon* shape)
{
	

	if (shape->radius == 0.0f)
	{
		// Put the ray into the polygon's frame of reference.
		Vec2 p1 = input->origin;
		Vec2 d = input->translation;

		float lower = 0.0f, upper = input->maxFraction;

		int32_t index = -1;

		b2RayCastOutput output = {0};

		for (int32_t i = 0; i < shape->count; ++i)
		{
			// p = p1 + a * d
			// dot(normal, p - v) = 0
			// dot(normal, p1 - v) + a * dot(normal, d) = 0
			float numerator = vec2_dot(shape->normals[i], vec2_sub(shape->vertices[i], p1));
			float denominator = vec2_dot(shape->normals[i], d);

			if (denominator == 0.0f)
			{
				if (numerator < 0.0f)
				{
					return output;
				}
			}
			else
			{
				// Note: we want this predicate without division:
				// lower < numerator / denominator, where denominator < 0
				// Since denominator < 0, we have to flip the inequality:
				// lower < numerator / denominator <==> denominator * lower > numerator.
				if (denominator < 0.0f && numerator < lower * denominator)
				{
					// Increase lower.
					// The segment enters this half-space.
					lower = numerator / denominator;
					index = i;
				}
				else if (denominator > 0.0f && numerator < upper * denominator)
				{
					// Decrease upper.
					// The segment exits this half-space.
					upper = numerator / denominator;
				}
			}

			// The use of epsilon here causes the B2_ASSERT on lower to trip
			// in some cases. Apparently the use of epsilon was to make edge
			// shapes work, but now those are handled separately.
			// if (upper < lower - b2_epsilon)
			if (upper < lower)
			{
				return output;
			}
		}

		

		if (index >= 0)
		{
			output.fraction = lower;
			output.normal = shape->normals[index];
			output.point = vec2_mul_add(p1, lower, d);
			output.hit = true;
		}

		return output;
	}

	// TODO_ERIN this is not working for ray vs box (zero radii)
	b2ShapeCastPairInput castInput;
	castInput.proxyA = b2MakeProxy(shape->vertices, shape->count, shape->radius);
	castInput.proxyB = b2MakeProxy(&input->origin, 1, 0.0f);
	castInput.transformA = tran2_identity;
	castInput.transformB = tran2_identity;
	castInput.translationB = input->translation;
	castInput.maxFraction = input->maxFraction;
	return b2ShapeCast(&castInput);
}

b2RayCastOutput b2ShapeCastCircle(const b2ShapeCastInput* input, const b2Circle* shape)
{
	b2ShapeCastPairInput pairInput;
	pairInput.proxyA = b2MakeProxy(&shape->point, 1, shape->radius);
	pairInput.proxyB = b2MakeProxy(input->points, input->count, input->radius);
	pairInput.transformA = tran2_identity;
	pairInput.transformB = tran2_identity;
	pairInput.translationB = input->translation;
	pairInput.maxFraction = input->maxFraction;

	b2RayCastOutput output = b2ShapeCast(&pairInput);
	return output;
}

b2RayCastOutput b2ShapeCastCapsule(const b2ShapeCastInput* input, const b2Capsule* shape)
{
	b2ShapeCastPairInput pairInput;
	pairInput.proxyA = b2MakeProxy(&shape->point1, 2, shape->radius);
	pairInput.proxyB = b2MakeProxy(input->points, input->count, input->radius);
	pairInput.transformA = tran2_identity;
	pairInput.transformB = tran2_identity;
	pairInput.translationB = input->translation;
	pairInput.maxFraction = input->maxFraction;

	b2RayCastOutput output = b2ShapeCast(&pairInput);
	return output;
}

b2RayCastOutput b2ShapeCastSegment(const b2ShapeCastInput* input, const b2Segment* shape)
{
	b2ShapeCastPairInput pairInput;
	pairInput.proxyA = b2MakeProxy(&shape->point1, 2, 0.0f);
	pairInput.proxyB = b2MakeProxy(input->points, input->count, input->radius);
	pairInput.transformA = tran2_identity;
	pairInput.transformB = tran2_identity;
	pairInput.translationB = input->translation;
	pairInput.maxFraction = input->maxFraction;

	b2RayCastOutput output = b2ShapeCast(&pairInput);
	return output;
}

b2RayCastOutput b2ShapeCastPolygon(const b2ShapeCastInput* input, const b2Polygon* shape)
{
	b2ShapeCastPairInput pairInput;
	pairInput.proxyA = b2MakeProxy(shape->vertices, shape->count, shape->radius);
	pairInput.proxyB = b2MakeProxy(input->points, input->count, input->radius);
	pairInput.transformA = tran2_identity;
	pairInput.transformB = tran2_identity;
	pairInput.translationB = input->translation;
	pairInput.maxFraction = input->maxFraction;

	b2RayCastOutput output = b2ShapeCast(&pairInput);
	return output;
}
