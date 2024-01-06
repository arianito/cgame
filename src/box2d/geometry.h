// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#pragma once

#include "box2d/constants.h"
#include "box2d/types.h"

typedef struct b2Hull b2Hull;
typedef struct b2CastOutput b2RayCastOutput;
typedef struct b2RayCastInput b2RayCastInput;

/// This holds the mass data computed for a shape.
typedef struct b2MassData
{
	/// The mass of the shape, usually in kilograms.
	float mass;

	/// The position of the shape's centroid relative to the shape's origin.
	Vec2 center;

	/// The rotational inertia of the shape about the local origin.
	float I;
} b2MassData;

/// A solid circle
typedef struct b2Circle
{
	Vec2 point;
	float radius;
} b2Circle;

/// A solid capsule
typedef struct b2Capsule
{
	Vec2 point1, point2;
	float radius;
} b2Capsule;

/// A solid convex polygon. It is assumed that the interior of the polygon is to
/// the left of each edge.
/// Polygons have a maximum number of vertices equal to b2_maxPolygonVertices.
/// In most cases you should not need many vertices for a convex polygon.
///	@warning DO NOT fill this out manually, instead use a helper function like
///	b2MakePolygon or b2MakeBox.
typedef struct b2Polygon
{
	Vec2 vertices[b2_maxPolygonVertices];
	Vec2 normals[b2_maxPolygonVertices];
	Vec2 centroid;
	float radius;
	int32_t count;
} b2Polygon;

/// A line segment with two-sided collision.
typedef struct b2Segment
{
	Vec2 point1, point2;
} b2Segment;

/// A smooth line segment with one-sided collision. Only collides on the right side.
/// Several of these are generated for a chain shape.
/// ghost1 -> point1 -> point2 -> ghost2
typedef struct b2SmoothSegment
{
	/// The tail ghost vertex
	Vec2 ghost1;

	/// The line segment
	b2Segment segment;

	/// The head ghost vertex
	Vec2 ghost2;

	/// The owning chain shape index (internal usage only)
	int32_t chainIndex;
} b2SmoothSegment;

/// Validate ray cast input data (NaN, etc)
bool b2IsValidRay(const b2RayCastInput* input);

/// Make a convex polygon from a convex hull. This will assert if the hull is not valid.
b2Polygon b2MakePolygon(const b2Hull* hull, float radius);

/// Make an offset convex polygon from a convex hull. This will assert if the hull is not valid.
b2Polygon b2MakeOffsetPolygon(const b2Hull* hull, float radius, Tran2 transform);

/// Make a square polygon, bypassing the need for a convex hull.
b2Polygon b2MakeSquare(float h);

/// Make a box (rectangle) polygon, bypassing the need for a convex hull.
b2Polygon b2MakeBox(float hx, float hy);

/// Make a rounded box, bypassing the need for a convex hull.
b2Polygon b2MakeRoundedBox(float hx, float hy, float radius);

/// Make an offset box, bypassing the need for a convex hull.
b2Polygon b2MakeOffsetBox(float hx, float hy, Vec2 center, float angle);

/// Transform a polygon. This is useful for transfering a shape from one body to another.
b2Polygon b2TransformPolygon(Tran2 transform, const b2Polygon* polygon);

/// Compute mass properties of a circle
b2MassData b2ComputeCircleMass(const b2Circle* shape, float density);

/// Compute mass properties of a capsule
b2MassData b2ComputeCapsuleMass(const b2Capsule* shape, float density);

/// Compute mass properties of a polygon
b2MassData b2ComputePolygonMass(const b2Polygon* shape, float density);

/// Compute the bounding box of a transformed circle
AABB b2ComputeCircleAABB(const b2Circle* shape, Tran2 transform);

/// Compute the bounding box of a transformed capsule
AABB b2ComputeCapsuleAABB(const b2Capsule* shape, Tran2 transform);

/// Compute the bounding box of a transformed polygon
AABB b2ComputePolygonAABB(const b2Polygon* shape, Tran2 transform);

/// Compute the bounding box of a transformed line segment
AABB b2ComputeSegmentAABB(const b2Segment* shape, Tran2 transform);

/// Test a point for overlap with a circle in local space
bool b2PointInCircle(Vec2 point, const b2Circle* shape);

/// Test a point for overlap with a capsule in local space
bool b2PointInCapsule(Vec2 point, const b2Capsule* shape);

/// Test a point for overlap with a convex polygon in local space
bool b2PointInPolygon(Vec2 point, const b2Polygon* shape);

/// Ray cast versus circle in shape local space. Initial overlap is treated as a miss.
b2RayCastOutput b2RayCastCircle(const b2RayCastInput* input, const b2Circle* shape);

/// Ray cast versus capsule in shape local space. Initial overlap is treated as a miss.
b2RayCastOutput b2RayCastCapsule(const b2RayCastInput* input, const b2Capsule* shape);

/// Ray cast versus segment in shape local space. Optionally treat the segment as one-sided with hits from
/// the left side being treated as a miss.
b2RayCastOutput b2RayCastSegment(const b2RayCastInput* input, const b2Segment* shape, bool oneSided);

/// Ray cast versus polygon in shape local space. Initial overlap is treated as a miss.
b2RayCastOutput b2RayCastPolygon(const b2RayCastInput* input, const b2Polygon* shape);

/// Shape cast versus a circle. Initial overlap is treated as a miss.
b2RayCastOutput b2ShapeCastCircle(const b2ShapeCastInput* input, const b2Circle* shape);

/// Shape cast versus a capsule. Initial overlap is treated as a miss.
b2RayCastOutput b2ShapeCastCapsule(const b2ShapeCastInput* input, const b2Capsule* shape);

/// Shape cast versus a line segment. Initial overlap is treated as a miss.
b2RayCastOutput b2ShapeCastSegment(const b2ShapeCastInput* input, const b2Segment* shape);

/// Shape cast versus a convex polygon. Initial overlap is treated as a miss.
b2RayCastOutput b2ShapeCastPolygon(const b2ShapeCastInput* input, const b2Polygon* shape);
