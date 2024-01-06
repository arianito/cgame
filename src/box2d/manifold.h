// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#pragma once

#include "box2d/types.h"

typedef struct b2Circle b2Circle;
typedef struct b2Capsule b2Capsule;
typedef struct b2DistanceCache b2DistanceCache;
typedef struct b2Polygon b2Polygon;
typedef struct b2Segment b2Segment;
typedef struct b2SmoothSegment b2SmoothSegment;

/// A manifold point is a contact point belonging to a contact
/// manifold. It holds details related to the geometry and dynamics
/// of the contact points.
typedef struct b2ManifoldPoint
{
	/// world coordinates of contact point
	Vec2 point;

	/// body anchors used by solver internally
	Vec2 anchorA, anchorB;

	/// the separation of the contact point, negative if penetrating
	float separation;

	/// the non-penetration impulse
	float normalImpulse;

	/// the friction impulse
	float tangentImpulse;

	/// uniquely identifies a contact point between two shapes
	uint16_t id;

	/// did this contact point exist the previous step?
	bool persisted;
} b2ManifoldPoint;

/// Contact manifold convex shapes.
typedef struct b2Manifold
{
	b2ManifoldPoint points[2];
	Vec2 normal;
	int32_t pointCount;
} b2Manifold;

/// Use this to initialize your manifold
static const b2Manifold b2_emptyManifold = B2_ZERO_INIT;

/// Compute the collision manifold between two circles.
b2Manifold b2CollideCircles(const b2Circle* circleA, Tran2 xfA, const b2Circle* circleB, Tran2 xfB);

/// Compute the collision manifold between a capsule and circle
b2Manifold b2CollideCapsuleAndCircle(const b2Capsule* capsuleA, Tran2 xfA, const b2Circle* circleB,
											   Tran2 xfB);

/// Compute the collision manifold between an segment and a circle.
b2Manifold b2CollideSegmentAndCircle(const b2Segment* segmentA, Tran2 xfA, const b2Circle* circleB,
											   Tran2 xfB);

/// Compute the collision manifold between a polygon and a circle.
b2Manifold b2CollidePolygonAndCircle(const b2Polygon* polygonA, Tran2 xfA, const b2Circle* circleB,
											   Tran2 xfB);

/// Compute the collision manifold between a capsule and circle
b2Manifold b2CollideCapsules(const b2Capsule* capsuleA, Tran2 xfA, const b2Capsule* capsuleB, Tran2 xfB,
									   b2DistanceCache* cache);

/// Compute the collision manifold between an segment and a capsule.
b2Manifold b2CollideSegmentAndCapsule(const b2Segment* segmentA, Tran2 xfA, const b2Capsule* capsuleB,
												Tran2 xfB, b2DistanceCache* cache);

/// Compute the collision manifold between a polygon and capsule
b2Manifold b2CollidePolygonAndCapsule(const b2Polygon* polygonA, Tran2 xfA, const b2Capsule* capsuleB,
												Tran2 xfB, b2DistanceCache* cache);

/// Compute the collision manifold between two polygons.
b2Manifold b2CollidePolygons(const b2Polygon* polyA, Tran2 xfA, const b2Polygon* polyB, Tran2 xfB,
									   b2DistanceCache* cache);

/// Compute the collision manifold between an segment and a polygon.
b2Manifold b2CollideSegmentAndPolygon(const b2Segment* segmentA, Tran2 xfA, const b2Polygon* polygonB,
												Tran2 xfB, b2DistanceCache* cache);

/// Compute the collision manifold between a smooth segment and a circle.
b2Manifold b2CollideSmoothSegmentAndCircle(const b2SmoothSegment* smoothSegmentA, Tran2 xfA,
													 const b2Circle* circleB, Tran2 xfB);

/// Compute the collision manifold between an segment and a capsule.
b2Manifold b2CollideSmoothSegmentAndCapsule(const b2SmoothSegment* segmentA, Tran2 xfA, const b2Capsule* capsuleB,
													  Tran2 xfB, b2DistanceCache* cache);

/// Compute the collision manifold between a smooth segment and a rounded polygon.
b2Manifold b2CollideSmoothSegmentAndPolygon(const b2SmoothSegment* segmentA, Tran2 xfA, const b2Polygon* polygonB,
													  Tran2 xfB, b2DistanceCache* cache);
