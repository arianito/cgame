
// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#include "box2d/distance.h"

#include "core.h"

#include "box2d/constants.h"
#include "box2d/timer.h"

#include "math/rot2.h"
#include "math/vec2.h"
#include "math/tran2.h"

#include <float.h>

#define B2_RESTRICT

Tran2 b2GetSweepTransform(const b2Sweep *sweep, float time)
{
	// https://fgiesen.wordpress.com/2012/08/15/linear-interpolation-past-present-and-future/
	Tran2 xf;
	xf.position = vec2_add(vec2_mulfv(1.0f - time, sweep->c1), vec2_mulfv(time, sweep->c2));
	float angle = (1.0f - time) * sweep->a1 + time * sweep->a2;
	xf.rotation = rot2f(angle);

	// Shift to origin
	xf.position = vec2_sub(xf.position, rot2_rotate(xf.rotation, sweep->localCenter));
	return xf;
}

/// Follows Ericson 5.1.9 Closest Points of Two Line Segments
b2SegmentDistanceResult b2SegmentDistance(Vec2 p1, Vec2 q1, Vec2 p2, Vec2 q2)
{
	b2SegmentDistanceResult result = {0};

	Vec2 d1 = vec2_sub(q1, p1);
	Vec2 d2 = vec2_sub(q2, p2);
	Vec2 r = vec2_sub(p1, p2);
	float dd1 = vec2_dot(d1, d1);
	float dd2 = vec2_dot(d2, d2);
	float rd2 = vec2_dot(r, d2);
	float rd1 = vec2_dot(r, d1);

	const float epsSqr = FLT_EPSILON * FLT_EPSILON;

	if (dd1 < epsSqr || dd2 < epsSqr)
	{
		// Handle all degeneracies
		if (dd1 >= epsSqr)
		{
			// Segment 2 is degenerate
			result.fraction1 = clampf(-rd1 / dd1, 0.0f, 1.0f);
			result.fraction2 = 0.0f;
		}
		else if (dd2 >= epsSqr)
		{
			// Segment 1 is degenerate
			result.fraction1 = 0.0f;
			result.fraction2 = clampf(rd2 / dd2, 0.0f, 1.0f);
		}
		else
		{
			result.fraction1 = 0.0f;
			result.fraction2 = 0.0f;
		}
	}
	else
	{
		// Non-degenerate segments
		float d12 = vec2_dot(d1, d2);

		float denom = dd1 * dd2 - d12 * d12;

		// Fraction on segment 1
		float f1 = 0.0f;
		if (denom != 0.0f)
		{
			// not parallel
			f1 = clampf((d12 * rd2 - rd1 * dd2) / denom, 0.0f, 1.0f);
		}

		// Compute point on segment 2 closest to p1 + f1 * d1
		float f2 = (d12 * f1 + rd2) / dd2;

		// Clamping of segment 2 requires a do over on segment 1
		if (f2 < 0.0f)
		{
			f2 = 0.0f;
			f1 = clampf(-rd1 / dd1, 0.0f, 1.0f);
		}
		else if (f2 > 1.0f)
		{
			f2 = 1.0f;
			f1 = clampf((d12 - rd1) / dd1, 0.0f, 1.0f);
		}

		result.fraction1 = f1;
		result.fraction2 = f2;
	}

	result.closest1 = vec2_mul_add(p1, result.fraction1, d1);
	result.closest2 = vec2_mul_add(p2, result.fraction2, d2);
	result.distanceSquared = b2DistanceSquared(result.closest1, result.closest2);
	return result;
}

// GJK using Voronoi regions (Christer Ericson) and Barycentric coordinates.

b2DistanceProxy b2MakeProxy(const Vec2 *vertices, int32_t count, float radius)
{
	count = minf(count, b2_maxPolygonVertices);
	b2DistanceProxy proxy;
	for (int32_t i = 0; i < count; ++i)
	{
		proxy.vertices[i] = vertices[i];
	}
	proxy.count = count;
	proxy.radius = radius;
	return proxy;
}

static Vec2 b2Weight2(float a1, Vec2 w1, float a2, Vec2 w2)
{
	return vec2(a1 * w1.x + a2 * w2.x, a1 * w1.y + a2 * w2.y);
}

static Vec2 b2Weight3(float a1, Vec2 w1, float a2, Vec2 w2, float a3, Vec2 w3)
{
	return vec2(a1 * w1.x + a2 * w2.x + a3 * w3.x, a1 * w1.y + a2 * w2.y + a3 * w3.y);
}

static int32_t b2FindSupport(const b2DistanceProxy *proxy, Vec2 direction)
{
	int32_t bestIndex = 0;
	float bestValue = vec2_dot(proxy->vertices[0], direction);
	for (int32_t i = 1; i < proxy->count; ++i)
	{
		float value = vec2_dot(proxy->vertices[i], direction);
		if (value > bestValue)
		{
			bestIndex = i;
			bestValue = value;
		}
	}

	return bestIndex;
}

typedef struct b2SimplexVertex
{
	Vec2 wA;		// support point in proxyA
	Vec2 wB;		// support point in proxyB
	Vec2 w;		// wB - wA
	float a;		// barycentric coordinate for closest point
	int32_t indexA; // wA index
	int32_t indexB; // wB index
} b2SimplexVertex;

typedef struct b2Simplex
{
	b2SimplexVertex v1, v2, v3;
	int32_t count;
} b2Simplex;

static float b2Simplex_Metric(const b2Simplex *s)
{
	switch (s->count)
	{
	case 0:

		return 0.0f;

	case 1:
		return 0.0f;

	case 2:
		return b2Distance(s->v1.w, s->v2.w);

	case 3:
		return vec2_cross(vec2_sub(s->v2.w, s->v1.w), vec2_sub(s->v3.w, s->v1.w));

	default:

		return 0.0f;
	}
}

static b2Simplex b2MakeSimplexFromCache(const b2DistanceCache *cache, const b2DistanceProxy *proxyA, Tran2 transformA,
										const b2DistanceProxy *proxyB, Tran2 transformB)
{

	b2Simplex s;

	// Copy data from cache.
	s.count = cache->count;

	b2SimplexVertex *vertices[] = {&s.v1, &s.v2, &s.v3};
	for (int32_t i = 0; i < s.count; ++i)
	{
		b2SimplexVertex *v = vertices[i];
		v->indexA = cache->indexA[i];
		v->indexB = cache->indexB[i];
		Vec2 wALocal = proxyA->vertices[v->indexA];
		Vec2 wBLocal = proxyB->vertices[v->indexB];
		v->wA = tran2_transform(transformA, wALocal);
		v->wB = tran2_transform(transformB, wBLocal);
		v->w = vec2_sub(v->wB, v->wA);

		// invalid
		v->a = -1.0f;
	}

	// If the cache is empty or invalid ...
	if (s.count == 0)
	{
		b2SimplexVertex *v = vertices[0];
		v->indexA = 0;
		v->indexB = 0;
		Vec2 wALocal = proxyA->vertices[0];
		Vec2 wBLocal = proxyB->vertices[0];
		v->wA = tran2_transform(transformA, wALocal);
		v->wB = tran2_transform(transformB, wBLocal);
		v->w = vec2_sub(v->wB, v->wA);
		v->a = 1.0f;
		s.count = 1;
	}

	return s;
}

static void b2MakeSimplexCache(b2DistanceCache *cache, const b2Simplex *simplex)
{
	cache->metric = b2Simplex_Metric(simplex);
	cache->count = (uint16_t)simplex->count;
	const b2SimplexVertex *vertices[] = {&simplex->v1, &simplex->v2, &simplex->v3};
	for (int32_t i = 0; i < simplex->count; ++i)
	{
		cache->indexA[i] = (uint8_t)vertices[i]->indexA;
		cache->indexB[i] = (uint8_t)vertices[i]->indexB;
	}
}

Vec2 b2ComputeSimplexSearchDirection(const b2Simplex *simplex)
{
	switch (simplex->count)
	{
	case 1:
		return vec2_neg(simplex->v1.w);

	case 2:
	{
		Vec2 e12 = vec2_sub(simplex->v2.w, simplex->v1.w);
		float sgn = vec2_cross(e12, vec2_neg(simplex->v1.w));
		if (sgn > 0.0f)
		{
			// Origin is left of e12.
			return vec2_crossfv(1.0f, e12);
		}
		else
		{
			// Origin is right of e12.
			return vec2_crossvf(e12, 1.0f);
		}
	}

	default:

		return vec2_zero;
	}
}

Vec2 b2ComputeSimplexClosestPoint(const b2Simplex *s)
{
	switch (s->count)
	{
	case 0:

		return vec2_zero;

	case 1:
		return s->v1.w;

	case 2:
		return b2Weight2(s->v1.a, s->v1.w, s->v2.a, s->v2.w);

	case 3:
		return vec2_zero;

	default:

		return vec2_zero;
	}
}

void b2ComputeSimplexWitnessPoints(Vec2 *a, Vec2 *b, const b2Simplex *s)
{
	switch (s->count)
	{
	case 0:

		break;

	case 1:
		*a = s->v1.wA;
		*b = s->v1.wB;
		break;

	case 2:
		*a = b2Weight2(s->v1.a, s->v1.wA, s->v2.a, s->v2.wA);
		*b = b2Weight2(s->v1.a, s->v1.wB, s->v2.a, s->v2.wB);
		break;

	case 3:
		*a = b2Weight3(s->v1.a, s->v1.wA, s->v2.a, s->v2.wA, s->v3.a, s->v3.wA);
		// TODO_ERIN why are these not equal?
		//*b = b2Weight3(s->v1.a, s->v1.wB, s->v2.a, s->v2.wB, s->v3.a, s->v3.wB);
		*b = *a;
		break;

	default:

		break;
	}
}

// Solve a line segment using barycentric coordinates.
//
// p = a1 * w1 + a2 * w2
// a1 + a2 = 1
//
// The vector from the origin to the closest point on the line is
// perpendicular to the line.
// e12 = w2 - w1
// dot(p, e) = 0
// a1 * dot(w1, e) + a2 * dot(w2, e) = 0
//
// 2-by-2 linear system
// [1      1     ][a1] = [1]
// [w1.e12 w2.e12][a2] = [0]
//
// Define
// d12_1 =  dot(w2, e12)
// d12_2 = -dot(w1, e12)
// d12 = d12_1 + d12_2
//
// Solution
// a1 = d12_1 / d12
// a2 = d12_2 / d12
void b2SolveSimplex2(b2Simplex *B2_RESTRICT s)
{
	Vec2 w1 = s->v1.w;
	Vec2 w2 = s->v2.w;
	Vec2 e12 = vec2_sub(w2, w1);

	// w1 region
	float d12_2 = -vec2_dot(w1, e12);
	if (d12_2 <= 0.0f)
	{
		// a2 <= 0, so we clamp it to 0
		s->v1.a = 1.0f;
		s->count = 1;
		return;
	}

	// w2 region
	float d12_1 = vec2_dot(w2, e12);
	if (d12_1 <= 0.0f)
	{
		// a1 <= 0, so we clamp it to 0
		s->v2.a = 1.0f;
		s->count = 1;
		s->v1 = s->v2;
		return;
	}

	// Must be in e12 region.
	float inv_d12 = 1.0f / (d12_1 + d12_2);
	s->v1.a = d12_1 * inv_d12;
	s->v2.a = d12_2 * inv_d12;
	s->count = 2;
}

void b2SolveSimplex3(b2Simplex *B2_RESTRICT s)
{
	Vec2 w1 = s->v1.w;
	Vec2 w2 = s->v2.w;
	Vec2 w3 = s->v3.w;

	// Edge12
	// [1      1     ][a1] = [1]
	// [w1.e12 w2.e12][a2] = [0]
	// a3 = 0
	Vec2 e12 = vec2_sub(w2, w1);
	float w1e12 = vec2_dot(w1, e12);
	float w2e12 = vec2_dot(w2, e12);
	float d12_1 = w2e12;
	float d12_2 = -w1e12;

	// Edge13
	// [1      1     ][a1] = [1]
	// [w1.e13 w3.e13][a3] = [0]
	// a2 = 0
	Vec2 e13 = vec2_sub(w3, w1);
	float w1e13 = vec2_dot(w1, e13);
	float w3e13 = vec2_dot(w3, e13);
	float d13_1 = w3e13;
	float d13_2 = -w1e13;

	// Edge23
	// [1      1     ][a2] = [1]
	// [w2.e23 w3.e23][a3] = [0]
	// a1 = 0
	Vec2 e23 = vec2_sub(w3, w2);
	float w2e23 = vec2_dot(w2, e23);
	float w3e23 = vec2_dot(w3, e23);
	float d23_1 = w3e23;
	float d23_2 = -w2e23;

	// Triangle123
	float n123 = vec2_cross(e12, e13);

	float d123_1 = n123 * vec2_cross(w2, w3);
	float d123_2 = n123 * vec2_cross(w3, w1);
	float d123_3 = n123 * vec2_cross(w1, w2);

	// w1 region
	if (d12_2 <= 0.0f && d13_2 <= 0.0f)
	{
		s->v1.a = 1.0f;
		s->count = 1;
		return;
	}

	// e12
	if (d12_1 > 0.0f && d12_2 > 0.0f && d123_3 <= 0.0f)
	{
		float inv_d12 = 1.0f / (d12_1 + d12_2);
		s->v1.a = d12_1 * inv_d12;
		s->v2.a = d12_2 * inv_d12;
		s->count = 2;
		return;
	}

	// e13
	if (d13_1 > 0.0f && d13_2 > 0.0f && d123_2 <= 0.0f)
	{
		float inv_d13 = 1.0f / (d13_1 + d13_2);
		s->v1.a = d13_1 * inv_d13;
		s->v3.a = d13_2 * inv_d13;
		s->count = 2;
		s->v2 = s->v3;
		return;
	}

	// w2 region
	if (d12_1 <= 0.0f && d23_2 <= 0.0f)
	{
		s->v2.a = 1.0f;
		s->count = 1;
		s->v1 = s->v2;
		return;
	}

	// w3 region
	if (d13_1 <= 0.0f && d23_1 <= 0.0f)
	{
		s->v3.a = 1.0f;
		s->count = 1;
		s->v1 = s->v3;
		return;
	}

	// e23
	if (d23_1 > 0.0f && d23_2 > 0.0f && d123_1 <= 0.0f)
	{
		float inv_d23 = 1.0f / (d23_1 + d23_2);
		s->v2.a = d23_1 * inv_d23;
		s->v3.a = d23_2 * inv_d23;
		s->count = 2;
		s->v1 = s->v3;
		return;
	}

	// Must be in triangle123
	float inv_d123 = 1.0f / (d123_1 + d123_2 + d123_3);
	s->v1.a = d123_1 * inv_d123;
	s->v2.a = d123_2 * inv_d123;
	s->v3.a = d123_3 * inv_d123;
	s->count = 3;
}

#define B2_GJK_DEBUG 0

// Warning: writing to these globals significantly slows multi-threading performance
#if B2_GJK_DEBUG
int32_t b2_gjkCalls;
int32_t b2_gjkIters;
int32_t b2_gjkMaxIters;
#endif

b2DistanceOutput b2ShapeDistance(b2DistanceCache *B2_RESTRICT cache, const b2DistanceInput *B2_RESTRICT input)
{
#if B2_GJK_DEBUG
	++b2_gjkCalls;
#endif

	b2DistanceOutput output = {0};

	const b2DistanceProxy *proxyA = &input->proxyA;
	const b2DistanceProxy *proxyB = &input->proxyB;

	Tran2 transformA = input->transformA;
	Tran2 transformB = input->transformB;

	// Initialize the simplex.
	b2Simplex simplex = b2MakeSimplexFromCache(cache, proxyA, transformA, proxyB, transformB);

	// Get simplex vertices as an array.
	b2SimplexVertex *vertices[] = {&simplex.v1, &simplex.v2, &simplex.v3};
	const int32_t k_maxIters = 20;

	// These store the vertices of the last simplex so that we
	// can check for duplicates and prevent cycling.
	int32_t saveA[3], saveB[3];
	int32_t saveCount = 0;

	// Main iteration loop.
	int32_t iter = 0;
	while (iter < k_maxIters)
	{
		// Copy simplex so we can identify duplicates.
		saveCount = simplex.count;
		for (int32_t i = 0; i < saveCount; ++i)
		{
			saveA[i] = vertices[i]->indexA;
			saveB[i] = vertices[i]->indexB;
		}

		switch (simplex.count)
		{
		case 1:
			break;

		case 2:
			b2SolveSimplex2(&simplex);
			break;

		case 3:
			b2SolveSimplex3(&simplex);
			break;

		default:
			break;
		}

		// If we have 3 points, then the origin is in the corresponding triangle.
		if (simplex.count == 3)
		{
			break;
		}

		// Get search direction.
		Vec2 d = b2ComputeSimplexSearchDirection(&simplex);

		// Ensure the search direction is numerically fit.
		if (vec2_dot(d, d) < FLT_EPSILON * FLT_EPSILON)
		{
			// The origin is probably contained by a line segment
			// or triangle. Thus the shapes are overlapped.

			// We can't return zero here even though there may be overlap.
			// In case the simplex is a point, segment, or triangle it is difficult
			// to determine if the origin is contained in the CSO or very close to it.
			break;
		}

		// Compute a tentative new simplex vertex using support points.
		b2SimplexVertex *vertex = vertices[simplex.count];
		vertex->indexA = b2FindSupport(proxyA, rot2_unrotate(transformA.rotation, vec2_neg(d)));
		vertex->wA = tran2_transform(transformA, proxyA->vertices[vertex->indexA]);
		vertex->indexB = b2FindSupport(proxyB, rot2_unrotate(transformB.rotation, d));
		vertex->wB = tran2_transform(transformB, proxyB->vertices[vertex->indexB]);
		vertex->w = vec2_sub(vertex->wB, vertex->wA);

		// Iteration count is equated to the number of support point calls.
		++iter;

#if B2_GJK_DEBUG
		++b2_gjkIters;
#endif

		// Check for duplicate support points. This is the main termination criteria.
		bool duplicate = false;
		for (int32_t i = 0; i < saveCount; ++i)
		{
			if (vertex->indexA == saveA[i] && vertex->indexB == saveB[i])
			{
				duplicate = true;
				break;
			}
		}

		// If we found a duplicate support point we must exit to avoid cycling.
		if (duplicate)
		{
			break;
		}

		// New vertex is ok and needed.
		++simplex.count;
	}

#if B2_GJK_DEBUG
	b2_gjkMaxIters = maxf(b2_gjkMaxIters, iter);
#endif

	// Prepare output
	b2ComputeSimplexWitnessPoints(&output.pointA, &output.pointB, &simplex);
	output.distance = b2Distance(output.pointA, output.pointB);
	output.iterations = iter;

	// Cache the simplex
	b2MakeSimplexCache(cache, &simplex);

	// Apply radii if requested
	if (input->useRadii)
	{
		if (output.distance < FLT_EPSILON)
		{
			// Shapes are too close to safely compute normal
			Vec2 p = (Vec2){0.5f * (output.pointA.x + output.pointB.x), 0.5f * (output.pointA.y + output.pointB.y)};
			output.pointA = p;
			output.pointB = p;
			output.distance = 0.0f;
		}
		else
		{
			// Keep closest points on perimeter even if overlapped, this way
			// the points move smoothly.
			float rA = proxyA->radius;
			float rB = proxyB->radius;
			output.distance = maxf(0.0f, output.distance - rA - rB);
			Vec2 normal = vec2_norm(vec2_sub(output.pointB, output.pointA));
			Vec2 offsetA = vec2(rA * normal.x, rA * normal.y);
			Vec2 offsetB = vec2(rB * normal.x, rB * normal.y);
			output.pointA = vec2_add(output.pointA, offsetA);
			output.pointB = vec2_sub(output.pointB, offsetB);
		}
	}

	return output;
}

// GJK-raycast
// Algorithm by Gino van den Bergen.
// "Smooth Mesh Contacts with GJK" in Game Physics Pearls. 2010
// TODO_ERIN this is failing when used to raycast a box
b2RayCastOutput b2ShapeCast(const b2ShapeCastPairInput *input)
{
	b2RayCastOutput output = {0};

	const b2DistanceProxy *proxyA = &input->proxyA;
	const b2DistanceProxy *proxyB = &input->proxyB;

	float radius = proxyA->radius + proxyB->radius;

	Tran2 xfA = input->transformA;
	Tran2 xfB = input->transformB;

	Vec2 r = input->translationB;
	Vec2 n = vec2_zero;
	float lambda = 0.0f;
	float maxFraction = input->maxFraction;

	// Initial simplex
	b2Simplex simplex;
	simplex.count = 0;

	// Get simplex vertices as an array.
	b2SimplexVertex *vertices[] = {&simplex.v1, &simplex.v2, &simplex.v3};

	// Get support point in -r direction
	int32_t indexA = b2FindSupport(proxyA, rot2_unrotate(xfA.rotation, vec2_neg(r)));
	Vec2 wA = tran2_transform(xfA, proxyA->vertices[indexA]);
	int32_t indexB = b2FindSupport(proxyB, rot2_unrotate(xfB.rotation, r));
	Vec2 wB = tran2_transform(xfB, proxyB->vertices[indexB]);
	Vec2 v = vec2_sub(wA, wB);

	// Sigma is the target distance between proxies
	const float sigma = maxf(b2_linearSlop, radius - b2_linearSlop);

	// Main iteration loop.
	const int32_t k_maxIters = 20;
	int32_t iter = 0;
	while (iter < k_maxIters && vec2_length(v) > sigma)
	{

		output.iterations += 1;

		// Support in direction -v (A - B)
		indexA = b2FindSupport(proxyA, rot2_unrotate(xfA.rotation, vec2_neg(v)));
		wA = tran2_transform(xfA, proxyA->vertices[indexA]);
		indexB = b2FindSupport(proxyB, rot2_unrotate(xfB.rotation, v));
		wB = tran2_transform(xfB, proxyB->vertices[indexB]);
		Vec2 p = vec2_sub(wA, wB);

		// -v is a normal at p
		v = vec2_norm(v);

		// Intersect ray with plane
		float vp = vec2_dot(v, p);
		float vr = vec2_dot(v, r);
		if (vp - sigma > lambda * vr)
		{
			if (vr <= 0.0f)
			{
				return output;
			}

			lambda = (vp - sigma) / vr;
			if (lambda > maxFraction)
			{
				return output;
			}

			n = vec2(-v.x, -v.y);
			simplex.count = 0;
		}

		// Reverse simplex since it works with B - A.
		// Shift by lambda * r because we want the closest point to the current clip point.
		// Note that the support point p is not shifted because we want the plane equation
		// to be formed in unshifted space.
		b2SimplexVertex *vertex = vertices[simplex.count];
		vertex->indexA = indexB;
		vertex->wA = vec2(wB.x + lambda * r.x, wB.y + lambda * r.y);
		vertex->indexB = indexA;
		vertex->wB = wA;
		vertex->w = vec2_sub(vertex->wB, vertex->wA);
		vertex->a = 1.0f;
		simplex.count += 1;

		switch (simplex.count)
		{
		case 1:
			break;

		case 2:
			b2SolveSimplex2(&simplex);
			break;

		case 3:
			b2SolveSimplex3(&simplex);
			break;
		default:
			break;
		}

		// If we have 3 points, then the origin is in the corresponding triangle.
		if (simplex.count == 3)
		{
			// Overlap
			return output;
		}

		// Get search direction.
		v = b2ComputeSimplexClosestPoint(&simplex);

		// Iteration count is equated to the number of support point calls.
		++iter;
	}

	if (iter == 0)
	{
		// Initial overlap
		return output;
	}

	// Prepare output.
	Vec2 pointA, pointB;
	b2ComputeSimplexWitnessPoints(&pointB, &pointA, &simplex);

	if (vec2_dot(v, v) > 0.0f)
	{
		n = vec2_norm(vec2_neg(v));
	}

	float radiusA = proxyA->radius;
	output.point = vec2(pointA.x + radiusA * n.x, pointA.y + radiusA * n.y);
	output.normal = n;
	output.fraction = lambda;
	output.iterations = iter;
	output.hit = true;
	return output;
}

#define B2_TOI_DEBUG 0

// Warning: writing to these globals significantly slows multi-threading performance
#if B2_TOI_DEBUG
float b2_toiTime, b2_toiMaxTime;
int32_t b2_toiCalls, b2_toiIters, b2_toiMaxIters;
int32_t b2_toiRootIters, b2_toiMaxRootIters;
#endif

typedef enum b2SeparationType
{
	b2_pointsType,
	b2_faceAType,
	b2_faceBType
} b2SeparationType;

typedef struct b2SeparationFunction
{
	const b2DistanceProxy *proxyA;
	const b2DistanceProxy *proxyB;
	b2Sweep sweepA, sweepB;
	Vec2 localPoint;
	Vec2 axis;
	b2SeparationType type;
} b2SeparationFunction;

b2SeparationFunction b2MakeSeparationFunction(const b2DistanceCache *cache, const b2DistanceProxy *proxyA, const b2Sweep *sweepA,
											  const b2DistanceProxy *proxyB, const b2Sweep *sweepB, float t1)
{
	b2SeparationFunction f;

	f.proxyA = proxyA;
	f.proxyB = proxyB;
	int32_t count = cache->count;

	f.sweepA = *sweepA;
	f.sweepB = *sweepB;

	Tran2 xfA = b2GetSweepTransform(sweepA, t1);
	Tran2 xfB = b2GetSweepTransform(sweepB, t1);

	if (count == 1)
	{
		f.type = b2_pointsType;
		Vec2 localPointA = proxyA->vertices[cache->indexA[0]];
		Vec2 localPointB = proxyB->vertices[cache->indexB[0]];
		Vec2 pointA = tran2_transform(xfA, localPointA);
		Vec2 pointB = tran2_transform(xfB, localPointB);
		f.axis = vec2_norm(vec2_sub(pointB, pointA));
		f.localPoint = vec2_zero;
		return f;
	}

	if (cache->indexA[0] == cache->indexA[1])
	{
		// Two points on B and one on A.
		f.type = b2_faceBType;
		Vec2 localPointB1 = proxyB->vertices[cache->indexB[0]];
		Vec2 localPointB2 = proxyB->vertices[cache->indexB[1]];

		f.axis = vec2_crossvf(vec2_sub(localPointB2, localPointB1), 1.0f);
		f.axis = vec2_norm(f.axis);
		Vec2 normal = rot2_rotate(xfB.rotation, f.axis);

		f.localPoint = vec2(0.5f * (localPointB1.x + localPointB2.x), 0.5f * (localPointB1.y + localPointB2.y));
		Vec2 pointB = tran2_transform(xfB, f.localPoint);

		Vec2 localPointA = proxyA->vertices[cache->indexA[0]];
		Vec2 pointA = tran2_transform(xfA, localPointA);

		float s = vec2_dot(vec2_sub(pointA, pointB), normal);
		if (s < 0.0f)
		{
			f.axis = vec2_neg(f.axis);
		}
		return f;
	}

	// Two points on A and one or two points on B.
	f.type = b2_faceAType;
	Vec2 localPointA1 = proxyA->vertices[cache->indexA[0]];
	Vec2 localPointA2 = proxyA->vertices[cache->indexA[1]];

	f.axis = vec2_crossvf(vec2_sub(localPointA2, localPointA1), 1.0f);
	f.axis = vec2_norm(f.axis);
	Vec2 normal = rot2_rotate(xfA.rotation, f.axis);

	f.localPoint = vec2(0.5f * (localPointA1.x + localPointA2.x), 0.5f * (localPointA1.y + localPointA2.y));
	Vec2 pointA = tran2_transform(xfA, f.localPoint);

	Vec2 localPointB = proxyB->vertices[cache->indexB[0]];
	Vec2 pointB = tran2_transform(xfB, localPointB);

	float s = vec2_dot(vec2_sub(pointB, pointA), normal);
	if (s < 0.0f)
	{
		f.axis = vec2_neg(f.axis);
	}
	return f;
}

float b2FindMinSeparation(const b2SeparationFunction *f, int32_t *indexA, int32_t *indexB, float t)
{
	Tran2 xfA = b2GetSweepTransform(&f->sweepA, t);
	Tran2 xfB = b2GetSweepTransform(&f->sweepB, t);

	switch (f->type)
	{
	case b2_pointsType:
	{
		Vec2 axisA = rot2_unrotate(xfA.rotation, f->axis);
		Vec2 axisB = rot2_unrotate(xfB.rotation, vec2_neg(f->axis));

		*indexA = b2FindSupport(f->proxyA, axisA);
		*indexB = b2FindSupport(f->proxyB, axisB);

		Vec2 localPointA = f->proxyA->vertices[*indexA];
		Vec2 localPointB = f->proxyB->vertices[*indexB];

		Vec2 pointA = tran2_transform(xfA, localPointA);
		Vec2 pointB = tran2_transform(xfB, localPointB);

		float separation = vec2_dot(vec2_sub(pointB, pointA), f->axis);
		return separation;
	}

	case b2_faceAType:
	{
		Vec2 normal = rot2_rotate(xfA.rotation, f->axis);
		Vec2 pointA = tran2_transform(xfA, f->localPoint);

		Vec2 axisB = rot2_unrotate(xfB.rotation, vec2_neg(normal));

		*indexA = -1;
		*indexB = b2FindSupport(f->proxyB, axisB);

		Vec2 localPointB = f->proxyB->vertices[*indexB];
		Vec2 pointB = tran2_transform(xfB, localPointB);

		float separation = vec2_dot(vec2_sub(pointB, pointA), normal);
		return separation;
	}

	case b2_faceBType:
	{
		Vec2 normal = rot2_rotate(xfB.rotation, f->axis);
		Vec2 pointB = tran2_transform(xfB, f->localPoint);

		Vec2 axisA = rot2_unrotate(xfA.rotation, vec2_neg(normal));

		*indexB = -1;
		*indexA = b2FindSupport(f->proxyA, axisA);

		Vec2 localPointA = f->proxyA->vertices[*indexA];
		Vec2 pointA = tran2_transform(xfA, localPointA);

		float separation = vec2_dot(vec2_sub(pointA, pointB), normal);
		return separation;
	}

	default:
		
		*indexA = -1;
		*indexB = -1;
		return 0.0f;
	}
}

//
float b2EvaluateSeparation(const b2SeparationFunction *f, int32_t indexA, int32_t indexB, float t)
{
	Tran2 xfA = b2GetSweepTransform(&f->sweepA, t);
	Tran2 xfB = b2GetSweepTransform(&f->sweepB, t);

	switch (f->type)
	{
	case b2_pointsType:
	{
		Vec2 localPointA = f->proxyA->vertices[indexA];
		Vec2 localPointB = f->proxyB->vertices[indexB];

		Vec2 pointA = tran2_transform(xfA, localPointA);
		Vec2 pointB = tran2_transform(xfB, localPointB);

		float separation = vec2_dot(vec2_sub(pointB, pointA), f->axis);
		return separation;
	}

	case b2_faceAType:
	{
		Vec2 normal = rot2_rotate(xfA.rotation, f->axis);
		Vec2 pointA = tran2_transform(xfA, f->localPoint);

		Vec2 localPointB = f->proxyB->vertices[indexB];
		Vec2 pointB = tran2_transform(xfB, localPointB);

		float separation = vec2_dot(vec2_sub(pointB, pointA), normal);
		return separation;
	}

	case b2_faceBType:
	{
		Vec2 normal = rot2_rotate(xfB.rotation, f->axis);
		Vec2 pointB = tran2_transform(xfB, f->localPoint);

		Vec2 localPointA = f->proxyA->vertices[indexA];
		Vec2 pointA = tran2_transform(xfA, localPointA);

		float separation = vec2_dot(vec2_sub(pointA, pointB), normal);
		return separation;
	}

	default:
		
		return 0.0f;
	}
}

// CCD via the local separating axis method. This seeks progression
// by computing the largest time at which separation is maintained.
b2TOIOutput b2TimeOfImpact(const b2TOIInput *input)
{
#if B2_TOI_DEBUG
	b2Timer timer = b2CreateTimer();
	++b2_toiCalls;
#endif

	b2TOIOutput output;
	output.state = b2_toiStateUnknown;
	output.t = input->tMax;

	const b2DistanceProxy *proxyA = &input->proxyA;
	const b2DistanceProxy *proxyB = &input->proxyB;

	b2Sweep sweepA = input->sweepA;
	b2Sweep sweepB = input->sweepB;

	// Large rotations can make the root finder fail, so normalize the sweep angles.
	float twoPi = 2.0f * b2_pi;
	{
		float d = twoPi * floorf(sweepA.a1 / twoPi);
		sweepA.a1 -= d;
		sweepA.a2 -= d;
	}
	{
		float d = twoPi * floorf(sweepB.a1 / twoPi);
		sweepB.a1 -= d;
		sweepB.a2 -= d;
	}

	float tMax = input->tMax;

	float totalRadius = proxyA->radius + proxyB->radius;
	float target = maxf(b2_linearSlop, totalRadius + b2_linearSlop);
	float tolerance = 0.25f * b2_linearSlop;
	

	float t1 = 0.0f;
	const int32_t k_maxIterations = 20;
	int32_t iter = 0;

	// Prepare input for distance query.
	b2DistanceCache cache = {0};
	b2DistanceInput distanceInput;
	distanceInput.proxyA = input->proxyA;
	distanceInput.proxyB = input->proxyB;
	distanceInput.useRadii = false;

	// The outer loop progressively attempts to compute new separating axes.
	// This loop terminates when an axis is repeated (no progress is made).
	for (;;)
	{
		Tran2 xfA = b2GetSweepTransform(&sweepA, t1);
		Tran2 xfB = b2GetSweepTransform(&sweepB, t1);

		// Get the distance between shapes. We can also use the results
		// to get a separating axis.
		distanceInput.transformA = xfA;
		distanceInput.transformB = xfB;
		b2DistanceOutput distanceOutput = b2ShapeDistance(&cache, &distanceInput);

		// If the shapes are overlapped, we give up on continuous collision.
		if (distanceOutput.distance <= 0.0f)
		{
			// Failure!
			output.state = b2_toiStateOverlapped;
			output.t = 0.0f;
			break;
		}

		if (distanceOutput.distance < target + tolerance)
		{
			// Victory!
			output.state = b2_toiStateHit;
			output.t = t1;
			break;
		}

		// Initialize the separating axis.
		b2SeparationFunction fcn = b2MakeSeparationFunction(&cache, proxyA, &sweepA, proxyB, &sweepB, t1);
#if 0
		// Dump the curve seen by the root finder
		{
			const int32_t N = 100;
			float dx = 1.0f / N;
			float xs[N + 1];
			float fs[N + 1];

			float x = 0.0f;

			for (int32_t i = 0; i <= N; ++i)
			{
				sweepA.GetTransform(&xfA, x);
				sweepB.GetTransform(&xfB, x);
				float f = fcn.Evaluate(xfA, xfB) - target;

				printf("%g %g\n", x, f);

				xs[i] = x;
				fs[i] = f;

				x += dx;
			}
		}
#endif

		// Compute the TOI on the separating axis. We do this by successively
		// resolving the deepest point. This loop is bounded by the number of vertices.
		bool done = false;
		float t2 = tMax;
		int32_t pushBackIter = 0;
		for (;;)
		{
			// Find the deepest point at t2. Store the witness point indices.
			int32_t indexA, indexB;
			float s2 = b2FindMinSeparation(&fcn, &indexA, &indexB, t2);

			// Is the final configuration separated?
			if (s2 > target + tolerance)
			{
				// Victory!
				output.state = b2_toiStateSeparated;
				output.t = tMax;
				done = true;
				break;
			}

			// Has the separation reached tolerance?
			if (s2 > target - tolerance)
			{
				// Advance the sweeps
				t1 = t2;
				break;
			}

			// Compute the initial separation of the witness points.
			float s1 = b2EvaluateSeparation(&fcn, indexA, indexB, t1);

			// Check for initial overlap. This might happen if the root finder
			// runs out of iterations.
			if (s1 < target - tolerance)
			{
				output.state = b2_toiStateFailed;
				output.t = t1;
				done = true;
				break;
			}

			// Check for touching
			if (s1 <= target + tolerance)
			{
				// Victory! t1 should hold the TOI (could be 0.0).
				output.state = b2_toiStateHit;
				output.t = t1;
				done = true;
				break;
			}

			// Compute 1D root of: f(x) - target = 0
			int32_t rootIterCount = 0;
			float a1 = t1, a2 = t2;
			for (;;)
			{
				// Use a mix of the secant rule and bisection.
				float t;
				if (rootIterCount & 1)
				{
					// Secant rule to improve convergence.
					t = a1 + (target - s1) * (a2 - a1) / (s2 - s1);
				}
				else
				{
					// Bisection to guarantee progress.
					t = 0.5f * (a1 + a2);
				}

				++rootIterCount;

#if B2_TOI_DEBUG
				++b2_toiRootIters;
#endif

				float s = b2EvaluateSeparation(&fcn, indexA, indexB, t);

				if (B2_ABS(s - target) < tolerance)
				{
					// t2 holds a tentative value for t1
					t2 = t;
					break;
				}

				// Ensure we continue to bracket the root.
				if (s > target)
				{
					a1 = t;
					s1 = s;
				}
				else
				{
					a2 = t;
					s2 = s;
				}

				if (rootIterCount == 50)
				{
					break;
				}
			}

#if B2_TOI_DEBUG
			b2_toiMaxRootIters = maxf(b2_toiMaxRootIters, rootIterCount);
#endif

			++pushBackIter;

			if (pushBackIter == b2_maxPolygonVertices)
			{
				break;
			}
		}

		++iter;
#if B2_TOI_DEBUG
		++b2_toiIters;
#endif

		if (done)
		{
			break;
		}

		if (iter == k_maxIterations)
		{
			// Root finder got stuck. Semi-victory.
			output.state = b2_toiStateFailed;
			output.t = t1;
			break;
		}
	}

#if B2_TOI_DEBUG
	b2_toiMaxIters = maxf(b2_toiMaxIters, iter);

	float time = b2GetMilliseconds(&timer);
	b2_toiMaxTime = maxf(b2_toiMaxTime, time);
	b2_toiTime += time;
#endif

	return output;
}
