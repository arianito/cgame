// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#include "box2d/manifold.h"

#include "core.h"

#include "box2d/distance.h"
#include "box2d/geometry.h"

#include <float.h>
#include <stdatomic.h>
#include <string.h>

#define B2_MAKE_ID(A, B) ((uint8_t)(A) << 8 | (uint8_t)(B))

static b2Polygon b2MakeCapsule(Vec2 p1, Vec2 p2, float radius)
{
	b2Polygon shape = {0};
	shape.vertices[0] = p1;
	shape.vertices[1] = p2;
	shape.centroid = vec2_lerp(p1, p2, 0.5f);

	Vec2 axis = vec2_norm(vec2_sub(p2, p1));
	Vec2 normal = vec2_perp_right(axis);

	shape.normals[0] = normal;
	shape.normals[1] = vec2_neg(normal);
	shape.count = 2;
	shape.radius = radius;

	return shape;
}

b2Manifold b2CollideCircles(const b2Circle* circleA, Tran2 xfA, const b2Circle* circleB, Tran2 xfB)
{
	b2Manifold manifold = {0};

	Vec2 pointA = tran2_transform(xfA, circleA->point);
	Vec2 pointB = tran2_transform(xfB, circleB->point);

	float distance;
	Vec2 normal = vec2_length_normal(&distance, vec2_sub(pointB, pointA));

	float radiusA = circleA->radius;
	float radiusB = circleB->radius;

	float separation = distance - radiusA - radiusB;
	if (separation > b2_speculativeDistance)
	{
		return manifold;
	}

	Vec2 cA = vec2_mul_add(pointA, radiusA, normal);
	Vec2 cB = vec2_mul_add(pointB, -radiusB, normal);
	manifold.normal = normal;
	manifold.points[0].point = vec2_lerp(cA, cB, 0.5f);
	manifold.points[0].separation = separation;
	manifold.points[0].id = 0;
	manifold.pointCount = 1;
	return manifold;
}

/// Compute the collision manifold between a capsule and circle
b2Manifold b2CollideCapsuleAndCircle(const b2Capsule* capsuleA, Tran2 xfA, const b2Circle* circleB, Tran2 xfB)
{
	b2Manifold manifold = {0};

	// Compute circle position in the frame of the capsule.
	Vec2 pB = tran2_untransform(xfA, tran2_transform(xfB, circleB->point));

	// Compute closest point
	Vec2 p1 = capsuleA->point1;
	Vec2 p2 = capsuleA->point2;

	Vec2 e = vec2_sub(p2, p1);

	// dot(p - pA, e) = 0
	// dot(p - (p1 + s1 * e), e) = 0
	// s1 = dot(p - p1, e)
	Vec2 pA;
	float s1 = vec2_dot(vec2_sub(pB, p1), e);
	float s2 = vec2_dot(vec2_sub(p2, pB), e);
	if (s1 < 0.0f)
	{
		// p1 region
		pA = p1;
	}
	else if (s2 < 0.0f)
	{
		// p2 region
		pA = p2;
	}
	else
	{
		// circle colliding with segment interior
		float s = s1 / vec2_dot(e, e);
		pA = vec2_mul_add(p1, s, e);
	}

	float distance;
	Vec2 normal = vec2_length_normal(&distance, vec2_sub(pB, pA));

	float radiusA = capsuleA->radius;
	float radiusB = circleB->radius;
	float separation = distance - radiusA - radiusB;
	if (separation > b2_speculativeDistance)
	{
		return manifold;
	}

	Vec2 cA = vec2_mul_add(pA, radiusA, normal);
	Vec2 cB = vec2_mul_add(pB, -radiusB, normal);
	manifold.normal = rot2_rotate(xfA.rotation, normal);
	manifold.points[0].point = tran2_transform(xfA, vec2_lerp(cA, cB, 0.5f));
	manifold.points[0].separation = separation;
	manifold.points[0].id = 0;
	manifold.pointCount = 1;
	return manifold;
}

b2Manifold b2CollidePolygonAndCircle(const b2Polygon* polygonA, Tran2 xfA, const b2Circle* circleB, Tran2 xfB)
{
	b2Manifold manifold = {0};

	// Compute circle position in the frame of the polygon.
	Vec2 c = tran2_untransform(xfA, tran2_transform(xfB, circleB->point));
	float radiusA = polygonA->radius;
	float radiusB = circleB->radius;
	float radius = radiusA + radiusB;

	// Find the min separating edge.
	int32_t normalIndex = 0;
	float separation = -FLT_MAX;
	int32_t vertexCount = polygonA->count;
	const Vec2* vertices = polygonA->vertices;
	const Vec2* normals = polygonA->normals;

	for (int32_t i = 0; i < vertexCount; ++i)
	{
		float s = vec2_dot(normals[i], vec2_sub(c, vertices[i]));
		if (s > separation)
		{
			separation = s;
			normalIndex = i;
		}
	}

	if (separation > radius + b2_speculativeDistance)
	{
		return manifold;
	}

	// Vertices of the reference edge.
	int32_t vertIndex1 = normalIndex;
	int32_t vertIndex2 = vertIndex1 + 1 < vertexCount ? vertIndex1 + 1 : 0;
	Vec2 v1 = vertices[vertIndex1];
	Vec2 v2 = vertices[vertIndex2];

	// Compute barycentric coordinates
	float u1 = vec2_dot(vec2_sub(c, v1), vec2_sub(v2, v1));
	float u2 = vec2_dot(vec2_sub(c, v2), vec2_sub(v1, v2));

	if (u1 < 0.0f && separation > FLT_EPSILON)
	{
		// Circle center is closest to v1 and safely outside the polygon
		Vec2 normal = vec2_norm(vec2_sub(c, v1));
		separation = vec2_dot(vec2_sub(c, v1), normal);
		if (separation > radius + b2_speculativeDistance)
		{
			return manifold;
		}

		Vec2 cA = vec2_mul_add(v1, radiusA, normal);
		Vec2 cB = vec2_mul_sub(c, radiusB, normal);
		manifold.normal = rot2_rotate(xfA.rotation, normal);
		manifold.points[0].point = tran2_transform(xfA, vec2_lerp(cA, cB, 0.5f));
		manifold.points[0].separation = vec2_dot(vec2_sub(cB, cA), normal);
		manifold.points[0].id = 0;
		manifold.pointCount = 1;
	}
	else if (u2 < 0.0f && separation > FLT_EPSILON)
	{
		// Circle center is closest to v2 and safely outside the polygon
		Vec2 normal = vec2_norm(vec2_sub(c, v2));
		separation = vec2_dot(vec2_sub(c, v2), normal);
		if (separation > radius + b2_speculativeDistance)
		{
			return manifold;
		}

		Vec2 cA = vec2_mul_add(v2, radiusA, normal);
		Vec2 cB = vec2_mul_sub(c, radiusB, normal);
		manifold.normal = rot2_rotate(xfA.rotation, normal);
		manifold.points[0].point = tran2_transform(xfA, vec2_lerp(cA, cB, 0.5f));
		manifold.points[0].separation = vec2_dot(vec2_sub(cB, cA), normal);
		manifold.points[0].id = 0;
		manifold.pointCount = 1;
	}
	else
	{
		// Circle center is between v1 and v2. Center may be inside polygon
		Vec2 normal = normals[normalIndex];
		manifold.normal = rot2_rotate(xfA.rotation, normal);

		// cA is the projection of the circle center onto to the reference edge
		Vec2 cA = vec2_mul_add(c, radiusA - vec2_dot(vec2_sub(c, v1), normal), normal);

		// cB is the deepest point on the circle with respect to the reference edge
		Vec2 cB = vec2_mul_sub(c, radiusB, normal);

		// The contact point is the midpoint in world space
		manifold.points[0].point = tran2_transform(xfA, vec2_lerp(cA, cB, 0.5f));
		manifold.points[0].separation = separation - radius;
		manifold.points[0].id = 0;
		manifold.pointCount = 1;
	}

	return manifold;
}

b2Manifold b2CollideCapsules(const b2Capsule* capsuleA, Tran2 xfA, const b2Capsule* capsuleB, Tran2 xfB,
							 b2DistanceCache* cache)
{
	b2Polygon polyA = b2MakeCapsule(capsuleA->point1, capsuleA->point2, capsuleA->radius);
	b2Polygon polyB = b2MakeCapsule(capsuleB->point1, capsuleB->point2, capsuleB->radius);
	return b2CollidePolygons(&polyA, xfA, &polyB, xfB, cache);
}

b2Manifold b2CollideSegmentAndCapsule(const b2Segment* segmentA, Tran2 xfA, const b2Capsule* capsuleB, Tran2 xfB,
									  b2DistanceCache* cache)
{
	b2Polygon polyA = b2MakeCapsule(segmentA->point1, segmentA->point2, 0.0f);
	b2Polygon polyB = b2MakeCapsule(capsuleB->point1, capsuleB->point2, capsuleB->radius);
	return b2CollidePolygons(&polyA, xfA, &polyB, xfB, cache);
}

b2Manifold b2CollidePolygonAndCapsule(const b2Polygon* polygonA, Tran2 xfA, const b2Capsule* capsuleB, Tran2 xfB,
									  b2DistanceCache* cache)
{
	b2Polygon polyB = b2MakeCapsule(capsuleB->point1, capsuleB->point2, capsuleB->radius);
	return b2CollidePolygons(polygonA, xfA, &polyB, xfB, cache);
}

// Polygon clipper used by GJK and SAT to compute contact points when there are potentially two contact points.
static b2Manifold b2ClipPolygons(const b2Polygon* polyA, Tran2 xfA, const b2Polygon* polyB, Tran2 xfB, int32_t edgeA,
								 int32_t edgeB, bool flip)
{
	b2Manifold manifold = {0};

	// reference polygon
	const b2Polygon* poly1;
	int32_t i11, i12;

	// incident polygon
	const b2Polygon* poly2;
	int32_t i21, i22;

	Tran2 xf;

	if (flip)
	{
		poly1 = polyB;
		poly2 = polyA;
		// take points in frame A into frame B
		xf = tran2_unmul(xfB, xfA);
		i11 = edgeB;
		i12 = edgeB + 1 < polyB->count ? edgeB + 1 : 0;
		i21 = edgeA;
		i22 = edgeA + 1 < polyA->count ? edgeA + 1 : 0;
	}
	else
	{
		poly1 = polyA;
		poly2 = polyB;
		// take points in frame B into frame A
		xf = tran2_unmul(xfA, xfB);
		i11 = edgeA;
		i12 = edgeA + 1 < polyA->count ? edgeA + 1 : 0;
		i21 = edgeB;
		i22 = edgeB + 1 < polyB->count ? edgeB + 1 : 0;
	}

	Vec2 normal = poly1->normals[i11];

	// Reference edge vertices
	Vec2 v11 = poly1->vertices[i11];
	Vec2 v12 = poly1->vertices[i12];

	// Incident edge vertices
	Vec2 v21 = tran2_transform(xf, poly2->vertices[i21]);
	Vec2 v22 = tran2_transform(xf, poly2->vertices[i22]);

	Vec2 tangent = vec2_crossfv(1.0f, normal);

	float lower1 = 0.0f;
	float upper1 = vec2_dot(vec2_sub(v12, v11), tangent);

	// Incident edge points opposite of tangent due to CCW winding
	float upper2 = vec2_dot(vec2_sub(v21, v11), tangent);
	float lower2 = vec2_dot(vec2_sub(v22, v11), tangent);

	// This check can fail slightly due to mismatch with GJK code.
	// Perhaps fallback to a single point here? Otherwise we get two coincident points.
	// if (upper2 < lower1 || upper1 < lower2)
	//{
	//	// numeric failure
	//	
	//	return manifold;
	//}

	Vec2 vLower;
	if (lower2 < lower1 && upper2 - lower2 > FLT_EPSILON)
	{
		vLower = vec2_lerp(v22, v21, (lower1 - lower2) / (upper2 - lower2));
	}
	else
	{
		vLower = v22;
	}

	Vec2 vUpper;
	if (upper2 > upper1 && upper2 - lower2 > FLT_EPSILON)
	{
		vUpper = vec2_lerp(v22, v21, (upper1 - lower2) / (upper2 - lower2));
	}
	else
	{
		vUpper = v21;
	}

	// TODO_ERIN vLower can be very close to vUpper, reduce to one point?

	float separationLower = vec2_dot(vec2_sub(vLower, v11), normal);
	float separationUpper = vec2_dot(vec2_sub(vUpper, v11), normal);

	float r1 = poly1->radius;
	float r2 = poly2->radius;

	// Put contact points at midpoint, accounting for radii
	vLower = vec2_mul_add(vLower, 0.5f * (r1 - r2 - separationLower), normal);
	vUpper = vec2_mul_add(vUpper, 0.5f * (r1 - r2 - separationUpper), normal);

	float radius = r1 + r2;

	if (flip == false)
	{
		manifold.normal = rot2_rotate(xfA.rotation, normal);
		b2ManifoldPoint* cp = manifold.points + 0;

		{
			cp->point = tran2_transform(xfA, vLower);
			cp->separation = separationLower - radius;
			cp->id = B2_MAKE_ID(i11, i22);
			manifold.pointCount += 1;
			cp += 1;
		}

		{
			cp->point = tran2_transform(xfA, vUpper);
			cp->separation = separationUpper - radius;
			cp->id = B2_MAKE_ID(i12, i21);
			manifold.pointCount += 1;
		}
	}
	else
	{
		manifold.normal = rot2_rotate(xfB.rotation, vec2_neg(normal));
		b2ManifoldPoint* cp = manifold.points + 0;

		{
			cp->point = tran2_transform(xfB, vUpper);
			cp->separation = separationUpper - radius;
			cp->id = B2_MAKE_ID(i21, i12);
			manifold.pointCount += 1;
			cp += 1;
		}

		{
			cp->point = tran2_transform(xfB, vLower);
			cp->separation = separationLower - radius;
			cp->id = B2_MAKE_ID(i22, i11);
			manifold.pointCount += 1;
		}
	}

	return manifold;
}

// Find the max separation between poly1 and poly2 using edge normals from poly1.
static float b2FindMaxSeparation(int32_t* edgeIndex, const b2Polygon* poly1, Tran2 xf1, const b2Polygon* poly2,
								 Tran2 xf2)
{
	int32_t count1 = poly1->count;
	int32_t count2 = poly2->count;
	const Vec2* n1s = poly1->normals;
	const Vec2* v1s = poly1->vertices;
	const Vec2* v2s = poly2->vertices;
	Tran2 xf = tran2_unmul(xf2, xf1);

	int32_t bestIndex = 0;
	float maxSeparation = -FLT_MAX;
	for (int32_t i = 0; i < count1; ++i)
	{
		// Get poly1 normal in frame2.
		Vec2 n = rot2_rotate(xf.rotation, n1s[i]);
		Vec2 v1 = tran2_transform(xf, v1s[i]);

		// Find deepest point for normal i.
		float si = FLT_MAX;
		for (int32_t j = 0; j < count2; ++j)
		{
			float sij = vec2_dot(n, vec2_sub(v2s[j], v1));
			if (sij < si)
			{
				si = sij;
			}
		}

		if (si > maxSeparation)
		{
			maxSeparation = si;
			bestIndex = i;
		}
	}

	*edgeIndex = bestIndex;
	return maxSeparation;
}

// This function assumes there is overlap
static b2Manifold b2PolygonSAT(const b2Polygon* polyA, Tran2 xfA, const b2Polygon* polyB, Tran2 xfB)
{
	int32_t edgeA = 0;
	float separationA = b2FindMaxSeparation(&edgeA, polyA, xfA, polyB, xfB);

	int32_t edgeB = 0;
	float separationB = b2FindMaxSeparation(&edgeB, polyB, xfB, polyA, xfA);

	bool flip;

	if (separationB > separationA)
	{
		flip = true;
		Vec2 normal = rot2_rotate(xfB.rotation, polyB->normals[edgeB]);
		Vec2 searchDirection = rot2_unrotate(xfA.rotation, normal);

		// Find the incident edge on polyA
		int32_t count = polyA->count;
		const Vec2* normals = polyA->normals;
		edgeA = 0;
		float minDot = FLT_MAX;
		for (int32_t i = 0; i < count; ++i)
		{
			float dot = vec2_dot(searchDirection, normals[i]);
			if (dot < minDot)
			{
				minDot = dot;
				edgeA = i;
			}
		}
	}
	else
	{
		flip = false;
		Vec2 normal = rot2_rotate(xfA.rotation, polyA->normals[edgeA]);
		Vec2 searchDirection = rot2_unrotate(xfB.rotation, normal);

		// Find the incident edge on polyB
		int32_t count = polyB->count;
		const Vec2* normals = polyB->normals;
		edgeB = 0;
		float minDot = FLT_MAX;
		for (int32_t i = 0; i < count; ++i)
		{
			float dot = vec2_dot(searchDirection, normals[i]);
			if (dot < minDot)
			{
				minDot = dot;
				edgeB = i;
			}
		}
	}

	return b2ClipPolygons(polyA, xfA, polyB, xfB, edgeA, edgeB, flip);
}

// Due to speculation, every polygon is rounded
// Algorithm:
// compute distance
// if distance <= 0.1f * b2_linearSlop
//   SAT
// else
//   find closest features from GJK
//   expect 2-1 or 1-1 or 1-2 features
//   if 2-1 or 1-2
//     clip
//   else
//     vertex-vertex
//   end
// end
b2Manifold b2CollidePolygons(const b2Polygon* polyA, Tran2 xfA, const b2Polygon* polyB, Tran2 xfB,
							 b2DistanceCache* cache)
{
	b2Manifold manifold = {0};
	float radius = polyA->radius + polyB->radius;

	b2DistanceInput input;
	input.proxyA = b2MakeProxy(polyA->vertices, polyA->count, 0.0f);
	input.proxyB = b2MakeProxy(polyB->vertices, polyB->count, 0.0f);
	input.transformA = xfA;
	input.transformB = xfB;
	input.useRadii = false;

	b2DistanceOutput output = b2ShapeDistance(cache, &input);

	if (output.distance > radius + b2_speculativeDistance)
	{
		return manifold;
	}

	if (output.distance < 0.1f * b2_linearSlop)
	{
		// distance is small or zero, fallback to SAT
		return b2PolygonSAT(polyA, xfA, polyB, xfB);
	}

	if (cache->count == 1)
	{
		// vertex-vertex collision
		Vec2 pA = output.pointA;
		Vec2 pB = output.pointB;

		float distance = output.distance;
		manifold.normal = vec2_norm(vec2_sub(pB, pA));
		b2ManifoldPoint* cp = manifold.points + 0;
		cp->point = vec2_mul_add(pB, 0.5f * (polyA->radius - polyB->radius - distance), manifold.normal);
		cp->separation = distance - radius;
		cp->id = B2_MAKE_ID(cache->indexA[0], cache->indexB[0]);
		manifold.pointCount = 1;
		return manifold;
	}

	// vertex-edge collision
	
	bool flip;
	int32_t countA = polyA->count;
	int32_t countB = polyB->count;
	int32_t edgeA, edgeB;

	int32_t a1 = cache->indexA[0];
	int32_t a2 = cache->indexA[1];
	int32_t b1 = cache->indexB[0];
	int32_t b2 = cache->indexB[1];

	if (a1 == a2)
	{
		// 1 point on A, expect 2 points on B
		

		// Find reference edge that most aligns with vector between closest points.
		// This works for capsules and polygons
		Vec2 axis = rot2_unrotate(xfB.rotation, vec2_sub(output.pointA, output.pointB));
		float dot1 = vec2_dot(axis, polyB->normals[b1]);
		float dot2 = vec2_dot(axis, polyB->normals[b2]);
		edgeB = dot1 > dot2 ? b1 : b2;

		flip = true;

		// Get the normal of the reference edge in polyA's frame.
		axis = rot2_unrotate(xfA.rotation, rot2_rotate(xfB.rotation, polyB->normals[edgeB]));

		// Find the incident edge on polyA
		// Limit search to edges adjacent to closest vertex on A
		int32_t edgeA1 = a1;
		int32_t edgeA2 = edgeA1 == 0 ? countA - 1 : edgeA1 - 1;
		dot1 = vec2_dot(axis, polyA->normals[edgeA1]);
		dot2 = vec2_dot(axis, polyA->normals[edgeA2]);
		edgeA = dot1 < dot2 ? edgeA1 : edgeA2;
	}
	else
	{
		// Find reference edge that most aligns with vector between closest points.
		// This works for capsules and polygons
		Vec2 axis = rot2_unrotate(xfA.rotation, vec2_sub(output.pointB, output.pointA));
		float dot1 = vec2_dot(axis, polyA->normals[a1]);
		float dot2 = vec2_dot(axis, polyA->normals[a2]);
		edgeA = dot1 > dot2 ? a1 : a2;

		flip = false;

		// Get the normal of the reference edge in polyB's frame.
		axis = rot2_unrotate(xfB.rotation, rot2_rotate(xfA.rotation, polyA->normals[edgeA]));

		// Find the incident edge on polyB
		// Limit search to edges adjacent to closest vertex
		int32_t edgeB1 = b1;
		int32_t edgeB2 = edgeB1 == 0 ? countB - 1 : edgeB1 - 1;
		dot1 = vec2_dot(axis, polyB->normals[edgeB1]);
		dot2 = vec2_dot(axis, polyB->normals[edgeB2]);
		edgeB = dot1 < dot2 ? edgeB1 : edgeB2;
	}

	return b2ClipPolygons(polyA, xfA, polyB, xfB, edgeA, edgeB, flip);
}

b2Manifold b2CollideSegmentAndCircle(const b2Segment* segmentA, Tran2 xfA, const b2Circle* circleB, Tran2 xfB)
{
	b2Capsule capsuleA = {segmentA->point1, segmentA->point2, 0.0f};
	return b2CollideCapsuleAndCircle(&capsuleA, xfA, circleB, xfB);
}

b2Manifold b2CollideSegmentAndPolygon(const b2Segment* segmentA, Tran2 xfA, const b2Polygon* polygonB, Tran2 xfB,
									  b2DistanceCache* cache)
{
	b2Polygon polygonA = b2MakeCapsule(segmentA->point1, segmentA->point2, 0.0f);
	return b2CollidePolygons(&polygonA, xfA, polygonB, xfB, cache);
}

b2Manifold b2CollideSmoothSegmentAndCircle(const b2SmoothSegment* segmentA, Tran2 xfA, const b2Circle* circleB,
										   Tran2 xfB)
{
	b2Manifold manifold = {0};

	// Compute circle in frame of segment
	Vec2 pB = tran2_untransform(xfA, tran2_transform(xfB, circleB->point));

	Vec2 p1 = segmentA->segment.point1;
	Vec2 p2 = segmentA->segment.point2;
	Vec2 e = vec2_sub(p2, p1);

	// Normal points to the right
	float offset = vec2_dot(vec2_perp_right(e), vec2_sub(pB, p1));
	if (offset < 0.0f)
	{
		// collision is one-sided
		return manifold;
	}

	// Barycentric coordinates
	float u = vec2_dot(e, vec2_sub(p2, pB));
	float v = vec2_dot(e, vec2_sub(pB, p1));

	Vec2 pA;

	if (v <= 0.0f)
	{
		// Behind point1?
		// Is pB in the voronoi region of the previous edge?
		Vec2 prevEdge = vec2_sub(p1, segmentA->ghost1);
		float uPrev = vec2_dot(prevEdge, vec2_sub(pB, p1));
		if (uPrev <= 0.0f)
		{
			return manifold;
		}

		pA = p1;
	}
	else if (u <= 0.0f)
	{
		// Ahead of point2?
		Vec2 nextEdge = vec2_sub(segmentA->ghost2, p2);
		float vNext = vec2_dot(nextEdge, vec2_sub(pB, p2));

		// Is pB in the voronoi region of the next edge?
		if (vNext > 0.0f)
		{
			return manifold;
		}

		pA = p2;
	}
	else
	{
		float ee = vec2_dot(e, e);
		pA = vec2(u * p1.x + v * p2.x, u * p1.y + v * p2.y);
		pA = ee > 0.0f ? vec2_mulfv(1.0f / ee, pA) : p1;
	}

	float distance;
	Vec2 normal = vec2_length_normal(&distance, vec2_sub(pB, pA));

	float radius = circleB->radius;
	float separation = distance - radius;
	if (separation > b2_speculativeDistance)
	{
		return manifold;
	}

	Vec2 cA = pA;
	Vec2 cB = vec2_mul_add(pB, -radius, normal);
	manifold.normal = rot2_rotate(xfA.rotation, normal);
	manifold.points[0].point = tran2_transform(xfA, vec2_lerp(cA, cB, 0.5f));
	manifold.points[0].separation = separation;
	manifold.points[0].id = 0;
	manifold.pointCount = 1;
	return manifold;
}

b2Manifold b2CollideSmoothSegmentAndCapsule(const b2SmoothSegment* segmentA, Tran2 xfA, const b2Capsule* capsuleB,
											Tran2 xfB, b2DistanceCache* cache)
{
	b2Polygon polyB = b2MakeCapsule(capsuleB->point1, capsuleB->point2, capsuleB->radius);
	return b2CollideSmoothSegmentAndPolygon(segmentA, xfA, &polyB, xfB, cache);
}

static b2Manifold b2ClipSegments(Vec2 a1, Vec2 a2, Vec2 b1, Vec2 b2, Vec2 normal, float ra, float rb, uint16_t id1,
								 uint16_t id2)
{
	b2Manifold manifold = {0};

	Vec2 tangent = vec2_perp_left(normal);

	// Barycentric coordinates of each point relative to a1 along tangent
	float lower1 = 0.0f;
	float upper1 = vec2_dot(vec2_sub(a2, a1), tangent);

	// Incident edge points opposite of tangent due to CCW winding
	float upper2 = vec2_dot(vec2_sub(b1, a1), tangent);
	float lower2 = vec2_dot(vec2_sub(b2, a1), tangent);

	// Do segments overlap?
	if (upper2 < lower1 || upper1 < lower2)
	{
		return manifold;
	}

	Vec2 vLower;
	if (lower2 < lower1 && upper2 - lower2 > FLT_EPSILON)
	{
		vLower = vec2_lerp(b2, b1, (lower1 - lower2) / (upper2 - lower2));
	}
	else
	{
		vLower = b2;
	}

	Vec2 vUpper;
	if (upper2 > upper1 && upper2 - lower2 > FLT_EPSILON)
	{
		vUpper = vec2_lerp(b2, b1, (upper1 - lower2) / (upper2 - lower2));
	}
	else
	{
		vUpper = b1;
	}

	// TODO_ERIN vLower can be very close to vUpper, reduce to one point?

	float separationLower = vec2_dot(vec2_sub(vLower, a1), normal);
	float separationUpper = vec2_dot(vec2_sub(vUpper, a1), normal);

	// Put contact points at midpoint, accounting for radii
	vLower = vec2_mul_add(vLower, 0.5f * (ra - rb - separationLower), normal);
	vUpper = vec2_mul_add(vUpper, 0.5f * (ra - rb - separationUpper), normal);

	float radius = ra + rb;

	manifold.normal = normal;
	{
		b2ManifoldPoint* cp = manifold.points + 0;
		cp->point = vLower;
		cp->separation = separationLower - radius;
		cp->id = id1;
	}

	{
		b2ManifoldPoint* cp = manifold.points + 1;
		cp->point = vUpper;
		cp->separation = separationUpper - radius;
		cp->id = id2;
	}

	manifold.pointCount = 2;

	return manifold;
}

enum b2NormalType
{
	// This means the normal points in a direction that is non-smooth relative to a convex vertex and should be skipped
	b2_normalSkip,

	// This means the normal points in a direction that is smooth relative to a convex vertex and should be used for collision
	b2_normalAdmit,

	// This means the normal is in a region of a concave vertex and should be snapped to the smooth segment normal
	b2_normalSnap
};

struct b2SmoothSegmentParams
{
	Vec2 edge1;
	Vec2 normal0;
	Vec2 normal2;
	bool convex1;
	bool convex2;
};

// Evaluate Guass map
// See https://box2d.org/posts/2020/06/ghost-collisions/
static enum b2NormalType b2ClassifyNormal(struct b2SmoothSegmentParams params, Vec2 normal)
{
	const float sinTol = 0.01f;

	if (vec2_dot(normal, params.edge1) <= 0.0f)
	{
		// Normal points towards the segment tail
		if (params.convex1)
		{
			if (vec2_cross(normal, params.normal0) > sinTol)
			{
				return b2_normalSkip;
			}

			return b2_normalAdmit;
		}
		else
		{
			return b2_normalSnap;
		}
	}
	else
	{
		// Normal points towards segment head
		if (params.convex2)
		{
			if (vec2_cross(params.normal2, normal) > sinTol)
			{
				return b2_normalSkip;
			}

			return b2_normalAdmit;
		}
		else
		{
			return b2_normalSnap;
		}
	}
}

b2Manifold b2CollideSmoothSegmentAndPolygon(const b2SmoothSegment* segmentA, Tran2 xfA, const b2Polygon* polygonB,
											Tran2 xfB, b2DistanceCache* cache)
{
	b2Manifold manifold = {0};

	Tran2 xf = tran2_unmul(xfA, xfB);

	Vec2 centroidB = tran2_transform(xf, polygonB->centroid);
	float radiusB = polygonB->radius;

	Vec2 p1 = segmentA->segment.point1;
	Vec2 p2 = segmentA->segment.point2;

	Vec2 edge1 = vec2_norm(vec2_sub(p2, p1));

	struct b2SmoothSegmentParams smoothParams;
	smoothParams.edge1 = edge1;

	const float convexTol = 0.01f;
	Vec2 edge0 = vec2_norm(vec2_sub(p1, segmentA->ghost1));
	smoothParams.normal0 = vec2_perp_right(edge0);
	smoothParams.convex1 = vec2_cross(edge0, edge1) >= convexTol;

	Vec2 edge2 = vec2_norm(vec2_sub(segmentA->ghost2, p2));
	smoothParams.normal2 = vec2_perp_right(edge2);
	smoothParams.convex2 = vec2_cross(edge1, edge2) >= convexTol;

	// Normal points to the right
	Vec2 normal1 = vec2_perp_right(edge1);
	bool behind1 = vec2_dot(normal1, vec2_sub(centroidB, p1)) < 0.0f;
	bool behind0 = true;
	bool behind2 = true;
	if (smoothParams.convex1)
	{
		behind0 = vec2_dot(smoothParams.normal0, vec2_sub(centroidB, p1)) < 0.0f;
	}

	if (smoothParams.convex2)
	{
		behind2 = vec2_dot(smoothParams.normal2, vec2_sub(centroidB, p2)) < 0.0f;
	}

	if (behind1 && behind0 && behind2)
	{
		// one-sided collision
		return manifold;
	}

	// Get polygonB in frameA
	int32_t count = polygonB->count;
	Vec2 vertices[b2_maxPolygonVertices];
	Vec2 normals[b2_maxPolygonVertices];
	// Vec2 sum = vec2_zero;
	for (int32_t i = 0; i < count; ++i)
	{
		vertices[i] = tran2_transform(xf, polygonB->vertices[i]);
		normals[i] = rot2_rotate(xf.rotation, polygonB->normals[i]);

		// sum = vec2_add(sum, vec2_sub(vertices[i], centroidB));
	}

	// float sumLength = vec2_length(sum);

	// Distance doesn't work correctly with partial polygons
	b2DistanceInput input;
	input.proxyA = b2MakeProxy(&segmentA->segment.point1, 2, 0.0f);
	input.proxyB = b2MakeProxy(vertices, count, 0.0f);
	input.transformA = tran2_identity;
	input.transformB = tran2_identity;
	input.useRadii = false;

	b2DistanceOutput output = b2ShapeDistance(cache, &input);

	if (output.distance > radiusB + b2_speculativeDistance)
	{
		return manifold;
	}

	// Snap concave normals for partial polygon
	Vec2 n0 = smoothParams.convex1 ? smoothParams.normal0 : normal1;
	Vec2 n2 = smoothParams.convex2 ? smoothParams.normal2 : normal1;

	// Index of incident vertex on polygon
	int32_t incidentIndex = -1;
	int32_t incidentNormal = -1;

	if (behind1 == false && output.distance > 0.1f * b2_linearSlop)
	{
		// The closest features may be two vertices or an edge and a vertex even when there should
		// be face contact

		if (cache->count == 1)
		{
			// vertex-vertex collision
			Vec2 pA = output.pointA;
			Vec2 pB = output.pointB;

			Vec2 normal = vec2_norm(vec2_sub(pB, pA));

			enum b2NormalType type = b2ClassifyNormal(smoothParams, normal);
			if (type == b2_normalSkip)
			{
				return manifold;
			}
			else if (type == b2_normalAdmit)
			{
				manifold.normal = rot2_rotate(xfA.rotation, normal);
				b2ManifoldPoint* cp = manifold.points + 0;
				cp->point = tran2_transform(xfA, pA);
				cp->separation = output.distance - radiusB;
				cp->id = B2_MAKE_ID(cache->indexA[0], cache->indexB[0]);
				manifold.pointCount = 1;
				return manifold;
			}

			// fall through b2_normalSnap
			incidentIndex = cache->indexB[0];
		}
		else
		{
			// vertex-edge collision
			

			int32_t ia1 = cache->indexA[0];
			int32_t ia2 = cache->indexA[1];
			int32_t ib1 = cache->indexB[0];
			int32_t ib2 = cache->indexB[1];

			if (ia1 == ia2)
			{
				// 1 point on A, expect 2 points on B
				

				// Find polygon normal most aligned with vector between closest points.
				// This effectively sorts ib1 and ib2
				Vec2 normalB = vec2_sub(output.pointA, output.pointB);
				float dot1 = vec2_dot(normalB, normals[ib1]);
				float dot2 = vec2_dot(normalB, normals[ib2]);
				int32_t ib = dot1 > dot2 ? ib1 : ib2;

				// Use accurate normal
				normalB = normals[ib];

				enum b2NormalType type = b2ClassifyNormal(smoothParams, vec2_neg(normalB));
				if (type == b2_normalSkip)
				{
					return manifold;
				}
				else if (type == b2_normalAdmit)
				{
					// Get polygon edge associated with normal
					ib1 = ib;
					ib2 = ib < count - 1 ? ib + 1 : 0;

					Vec2 b1 = vertices[ib1];
					Vec2 b2 = vertices[ib2];

					// Find incident segment vertex
					dot1 = vec2_dot(normalB, vec2_sub(p1, b1));
					dot2 = vec2_dot(normalB, vec2_sub(p2, b1));

					if (dot1 < dot2)
					{
						if (vec2_dot(n0, normalB) < vec2_dot(normal1, normalB))
						{
							// Neighbor is incident
							return manifold;
						}
					}
					else
					{
						if (vec2_dot(n2, normalB) < vec2_dot(normal1, normalB))
						{
							// Neighbor is incident
							return manifold;
						}
					}

					manifold = b2ClipSegments(b1, b2, p1, p2, normalB, radiusB, 0.0f, B2_MAKE_ID(ib1, 1), B2_MAKE_ID(ib2, 0));
					manifold.normal = rot2_rotate(xfA.rotation, vec2_neg(normalB));
					manifold.points[0].point = tran2_transform(xfA, manifold.points[0].point);
					manifold.points[1].point = tran2_transform(xfA, manifold.points[1].point);
					return manifold;
				}

				// fall through b2_normalSnap
				incidentNormal = ib;
			}
			else
			{
				// Get index of incident polygonB vertex
				float dot1 = vec2_dot(normal1, vec2_sub(vertices[ib1], p1));
				float dot2 = vec2_dot(normal1, vec2_sub(vertices[ib2], p2));
				incidentIndex = dot1 < dot2 ? ib1 : ib2;
			}
		}
	}
	else
	{
		// SAT edge normal
		float edgeSeparation = FLT_MAX;

		for (int32_t i = 0; i < count; ++i)
		{
			float s = vec2_dot(normal1, vec2_sub(vertices[i], p1));
			if (s < edgeSeparation)
			{
				edgeSeparation = s;
				incidentIndex = i;
			}
		}

		// Check convex neighbor for edge separation
		if (smoothParams.convex1)
		{
			float s0 = FLT_MAX;

			for (int32_t i = 0; i < count; ++i)
			{
				float s = vec2_dot(smoothParams.normal0, vec2_sub(vertices[i], p1));
				if (s < s0)
				{
					s0 = s;
				}
			}

			if (s0 > edgeSeparation)
			{
				edgeSeparation = s0;

				// Indicate neighbor owns edge separation
				incidentIndex = -1;
			}
		}

		// Check convex neighbor for edge separation
		if (smoothParams.convex2)
		{
			float s2 = FLT_MAX;

			for (int32_t i = 0; i < count; ++i)
			{
				float s = vec2_dot(smoothParams.normal2, vec2_sub(vertices[i], p2));
				if (s < s2)
				{
					s2 = s;
				}
			}

			if (s2 > edgeSeparation)
			{
				edgeSeparation = s2;

				// Indicate neighbor owns edge separation
				incidentIndex = -1;
			}
		}

		// SAT polygon normals
		float polygonSeparation = -FLT_MAX;
		int32_t referenceIndex = -1;

		for (int32_t i = 0; i < count; ++i)
		{
			Vec2 n = normals[i];

			// Check the infinite sides of the partial polygon
			if ((smoothParams.convex1 && vec2_cross(n0, n) > 0.0f) || (smoothParams.convex2 && vec2_cross(n, n2) > 0.0f))
			{
				continue;
			}

			Vec2 p = vertices[i];
			float s = minf(vec2_dot(n, vec2_sub(p2, p)), vec2_dot(n, vec2_sub(p1, p)));

			if (s > polygonSeparation)
			{
				polygonSeparation = s;
				referenceIndex = i;
			}
		}

		if (polygonSeparation > edgeSeparation)
		{
			int32_t ia1 = referenceIndex;
			int32_t ia2 = ia1 < count - 1 ? ia1 + 1 : 0;
			Vec2 a1 = vertices[ia1];
			Vec2 a2 = vertices[ia2];

			Vec2 n = normals[ia1];

			float dot1 = vec2_dot(n, vec2_sub(p1, a1));
			float dot2 = vec2_dot(n, vec2_sub(p2, a1));

			if (dot1 < dot2)
			{
				if (vec2_dot(n0, n) < vec2_dot(normal1, n))
				{
					// Neighbor is incident
					return manifold;
				}
			}
			else
			{
				if (vec2_dot(n2, n) < vec2_dot(normal1, n))
				{
					// Neighbor is incident
					return manifold;
				}
			}

			manifold = b2ClipSegments(a1, a2, p1, p2, normals[ia1], radiusB, 0.0f, B2_MAKE_ID(ia1, 1), B2_MAKE_ID(ia2, 0));
			manifold.normal = rot2_rotate(xfA.rotation, vec2_neg(normals[ia1]));
			manifold.points[0].point = tran2_transform(xfA, manifold.points[0].point);
			manifold.points[1].point = tran2_transform(xfA, manifold.points[1].point);
			return manifold;
		}

		if (incidentIndex == -1)
		{
			// neighboring segment is the separating axis
			return manifold;
		}

		// fall through segment normal axis
	}

	

	// Segment normal

	// Find incident polygon normal: normal adjacent to deepest vertex that is most anti-parallel to segment normal
	Vec2 b1, b2;
	int32_t ib1, ib2;

	if (incidentNormal != -1)
	{
		ib1 = incidentNormal;
		ib2 = ib1 < count - 1 ? ib1 + 1 : 0;
		b1 = vertices[ib1];
		b2 = vertices[ib2];
	}
	else
	{
		int32_t i2 = incidentIndex;
		int32_t i1 = i2 > 0 ? i2 - 1 : count - 1;
		float d1 = vec2_dot(normal1, normals[i1]);
		float d2 = vec2_dot(normal1, normals[i2]);
		if (d1 < d2)
		{
			ib1 = i1, ib2 = i2;
			b1 = vertices[ib1];
			b2 = vertices[ib2];
		}
		else
		{
			ib1 = i2, ib2 = i2 < count - 1 ? i2 + 1 : 0;
			b1 = vertices[ib1];
			b2 = vertices[ib2];
		}
	}

	manifold = b2ClipSegments(p1, p2, b1, b2, normal1, 0.0f, radiusB, B2_MAKE_ID(0, ib2), B2_MAKE_ID(1, ib1));
	manifold.normal = rot2_rotate(xfA.rotation, manifold.normal);
	manifold.points[0].point = tran2_transform(xfA, manifold.points[0].point);
	manifold.points[1].point = tran2_transform(xfA, manifold.points[1].point);

	return manifold;
}
