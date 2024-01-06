// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#include "shape.h"

#include "body.h"
#include "broad_phase.h"
#include "contact.h"
#include "world.h"

// needed for dll export
#include "box2d/box2d.h"
#include "box2d/event_types.h"

AABB b2ComputeShapeAABB(const b2Shape* shape, Tran2 xf)
{
	switch (shape->type)
	{
		case b2_capsuleShape:
			return b2ComputeCapsuleAABB(&shape->capsule, xf);
		case b2_circleShape:
			return b2ComputeCircleAABB(&shape->circle, xf);
		case b2_polygonShape:
			return b2ComputePolygonAABB(&shape->polygon, xf);
		case b2_segmentShape:
			return b2ComputeSegmentAABB(&shape->segment, xf);
		case b2_smoothSegmentShape:
			return b2ComputeSegmentAABB(&shape->smoothSegment.segment, xf);
		default:
		{
			
			AABB empty = {xf.position, xf.position};
			return empty;
		}
	}
}

Vec2 b2GetShapeCentroid(const b2Shape* shape)
{
	switch (shape->type)
	{
		case b2_capsuleShape:
			return vec2_lerp(shape->capsule.point1, shape->capsule.point2, 0.5f);
		case b2_circleShape:
			return shape->circle.point;
		case b2_polygonShape:
			return shape->polygon.centroid;
		case b2_segmentShape:
			return vec2_lerp(shape->segment.point1, shape->segment.point2, 0.5f);
		case b2_smoothSegmentShape:
			return vec2_lerp(shape->smoothSegment.segment.point1, shape->smoothSegment.segment.point2, 0.5f);
		default:
			return vec2_zero;
	}
}

b2MassData b2ComputeShapeMass(const b2Shape* shape)
{
	switch (shape->type)
	{
		case b2_capsuleShape:
			return b2ComputeCapsuleMass(&shape->capsule, shape->density);
		case b2_circleShape:
			return b2ComputeCircleMass(&shape->circle, shape->density);
		case b2_polygonShape:
			return b2ComputePolygonMass(&shape->polygon, shape->density);
		default:
		{
			return (b2MassData){0};
		}
	}
}

b2ShapeExtent b2ComputeShapeExtent(const b2Shape* shape)
{
	b2ShapeExtent extent = {0};

	switch (shape->type)
	{
		case b2_capsuleShape:
		{
			float radius = shape->capsule.radius;
			extent.minExtent = radius;
			extent.maxExtent = maxf(vec2_length(shape->capsule.point1), vec2_length(shape->capsule.point2)) + radius;
		}
		break;

		case b2_circleShape:
		{
			float radius = shape->circle.radius;
			extent.minExtent = radius;
			extent.maxExtent = vec2_length(shape->circle.point) + radius;
		}
		break;

		case b2_polygonShape:
		{
			const b2Polygon* poly = &shape->polygon;
			float minExtent = b2_huge;
			float maxExtent = 0.0f;
			int32_t count = poly->count;
			for (int32_t i = 0; i < count; ++i)
			{
				float planeOffset = vec2_dot(poly->normals[i], vec2_sub(poly->vertices[i], poly->centroid));
				minExtent = minf(minExtent, planeOffset);

				float distanceSqr = vec2_sqr_length(poly->vertices[i]);
				maxExtent = maxf(maxExtent, distanceSqr);
			}

			extent.minExtent = minExtent + poly->radius;
			extent.maxExtent = maxExtent + poly->radius;
		}
		break;

		default:
			break;
	}

	return extent;
}

b2RayCastOutput b2RayCastShape(const b2RayCastInput* input, const b2Shape* shape, Tran2 xf)
{
	b2RayCastInput localInput = *input;
	localInput.origin = tran2_untransform(xf, input->origin);
	localInput.translation = rot2_unrotate(xf.rotation, input->translation);

	b2RayCastOutput output = {0};
	switch (shape->type)
	{
		case b2_capsuleShape:
			output = b2RayCastCapsule(&localInput, &shape->capsule);
			break;
		case b2_circleShape:
			output = b2RayCastCircle(&localInput, &shape->circle);
			break;
		case b2_polygonShape:
			output = b2RayCastPolygon(&localInput, &shape->polygon);
			break;
		case b2_segmentShape:
			output = b2RayCastSegment(&localInput, &shape->segment, false);
			break;
		case b2_smoothSegmentShape:
			output = b2RayCastSegment(&localInput, &shape->smoothSegment.segment, true);
			break;
		default:
			return output;
	}

	output.point = tran2_transform(xf, output.point);
	output.normal = rot2_rotate(xf.rotation, output.normal);
	return output;
}

b2RayCastOutput b2ShapeCastShape(const b2ShapeCastInput* input, const b2Shape* shape, Tran2 xf)
{
	b2ShapeCastInput localInput = *input;

	for (int i = 0; i < localInput.count; ++i)
	{
		localInput.points[i] = tran2_untransform(xf, input->points[i]);
	}

	localInput.translation = rot2_unrotate(xf.rotation, input->translation);

	b2RayCastOutput output = {0};
	switch (shape->type)
	{
		case b2_capsuleShape:
			output = b2ShapeCastCapsule(&localInput, &shape->capsule);
			break;
		case b2_circleShape:
			output = b2ShapeCastCircle(&localInput, &shape->circle);
			break;
		case b2_polygonShape:
			output = b2ShapeCastPolygon(&localInput, &shape->polygon);
			break;
		case b2_segmentShape:
			output = b2ShapeCastSegment(&localInput, &shape->segment);
			break;
		case b2_smoothSegmentShape:
			output = b2ShapeCastSegment(&localInput, &shape->smoothSegment.segment);
			break;
		default:
			return output;
	}

	output.point = tran2_transform(xf, output.point);
	output.normal = rot2_rotate(xf.rotation, output.normal);
	return output;
}

void b2CreateShapeProxy(b2Shape* shape, b2BroadPhase* bp, b2BodyType type, Tran2 xf)
{
	

	// Create proxies in the broad-phase.
	shape->aabb = b2ComputeShapeAABB(shape, xf);

	// Smaller margin for static bodies. Cannot be zero due to TOI tolerance.
	float margin = type == b2_staticBody ? 4.0f * b2_linearSlop : b2_aabbMargin;
	shape->fatAABB.min.x = shape->aabb.min.x - margin;
	shape->fatAABB.min.y = shape->aabb.min.y - margin;
	shape->fatAABB.max.x = shape->aabb.max.x + margin;
	shape->fatAABB.max.y = shape->aabb.max.y + margin;

	shape->proxyKey = b2BroadPhase_CreateProxy(bp, type, shape->fatAABB, shape->filter.categoryBits, shape->object.index);
	
}

void b2DestroyShapeProxy(b2Shape* shape, b2BroadPhase* bp)
{
	if (shape->proxyKey != B2_NULL_INDEX)
	{
		b2BroadPhase_DestroyProxy(bp, shape->proxyKey);
		shape->proxyKey = B2_NULL_INDEX;
	}
}

b2DistanceProxy b2MakeShapeDistanceProxy(const b2Shape* shape)
{
	switch (shape->type)
	{
		case b2_capsuleShape:
			return b2MakeProxy(&shape->capsule.point1, 2, shape->capsule.radius);
		case b2_circleShape:
			return b2MakeProxy(&shape->circle.point, 1, shape->circle.radius);
		case b2_polygonShape:
			return b2MakeProxy(shape->polygon.vertices, shape->polygon.count, shape->polygon.radius);
		case b2_segmentShape:
			return b2MakeProxy(&shape->segment.point1, 2, 0.0f);
		case b2_smoothSegmentShape:
			return b2MakeProxy(&shape->smoothSegment.segment.point1, 2, 0.0f);
		default:
		{
			
			b2DistanceProxy empty = {0};
			return empty;
		}
	}
}

b2Shape* b2GetShape(b2World* world, b2ShapeId shapeId)
{
	
	b2Shape* shape = world->shapes + shapeId.index;
	
	
	return shape;
}

b2BodyId b2Shape_GetBody(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);

	b2Body* body = world->bodies + shape->bodyIndex;
	

	b2BodyId bodyId = {body->object.index, shapeId.world, body->object.revision};
	return bodyId;
}

void* b2Shape_GetUserData(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);
	return shape->userData;
}

bool b2Shape_IsSensor(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);
	return shape->isSensor;
}

bool b2Shape_TestPoint(b2ShapeId shapeId, Vec2 point)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	
	b2Shape* shape = world->shapes + shapeId.index;
	

	
	b2Body* body = world->bodies + shape->bodyIndex;
	

	Vec2 localPoint = tran2_untransform(body->transform, point);

	switch (shape->type)
	{
		case b2_capsuleShape:
			return b2PointInCapsule(localPoint, &shape->capsule);

		case b2_circleShape:
			return b2PointInCircle(localPoint, &shape->circle);

		case b2_polygonShape:
			return b2PointInPolygon(localPoint, &shape->polygon);

		default:
			return false;
	}
}

void b2Shape_SetDensity(b2ShapeId shapeId, float density)
{
	

	b2World* world = b2GetWorldFromIndex(shapeId.world);
	
	if (world->locked)
	{
		return;
	}
	
	b2Shape* shape = b2GetShape(world, shapeId);
	if (density == shape->density)
	{
		// early return to avoid expensive function
		return;
	}

	shape->density = density;

	b2Body* body = world->bodies + shape->bodyIndex;
	

	b2UpdateBodyMassData(world, body);
}

float b2Shape_GetDensity(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);
	return shape->density;
}

void b2Shape_SetFriction(b2ShapeId shapeId, float friction)
{
	

	b2World* world = b2GetWorldFromIndex(shapeId.world);
	
	if (world->locked)
	{
		return;
	}

	b2Shape* shape = b2GetShape(world, shapeId);
	shape->friction = friction;
}

float b2Shape_GetFriction(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);
	return shape->friction;
}

void b2Shape_SetRestitution(b2ShapeId shapeId, float restitution)
{
	

	b2World* world = b2GetWorldFromIndex(shapeId.world);
	
	if (world->locked)
	{
		return;
	}

	b2Shape* shape = b2GetShape(world, shapeId);
	shape->restitution = restitution;
}

float b2Shape_GetRestitution(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);
	return shape->restitution;
}

b2Filter b2Shape_GetFilter(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);
	return shape->filter;
}

void b2Shape_SetFilter(b2ShapeId shapeId, b2Filter filter)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);
	shape->filter = filter;

	b2Body* body = world->bodies + shape->bodyIndex;
	

	// Destroy any contacts associated with the shape
	int32_t contactKey = body->contactList;
	while (contactKey != B2_NULL_INDEX)
	{
		int32_t contactIndex = contactKey >> 1;
		int32_t edgeIndex = contactKey & 1;

		b2Contact* contact = world->contacts + contactIndex;
		contactKey = contact->edges[edgeIndex].nextKey;

		if (contact->shapeIndexA == shapeId.index || contact->shapeIndexB == shapeId.index)
		{
			b2DestroyContact(world, contact);
		}
	}

	if (body->isEnabled)
	{
		b2DestroyShapeProxy(shape, &world->broadPhase);
		b2CreateShapeProxy(shape, &world->broadPhase, body->type, body->transform);
	}
	else
	{
		
	}
}

b2ShapeType b2Shape_GetType(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);
	return shape->type;
}

const b2Circle* b2Shape_GetCircle(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);
	
	return &shape->circle;
}

const b2Segment* b2Shape_GetSegment(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);
	
	return &shape->segment;
}

const b2SmoothSegment* b2Shape_GetSmoothSegment(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);
	
	return &shape->smoothSegment;
}

const b2Capsule* b2Shape_GetCapsule(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);
	
	return &shape->capsule;
}

const b2Polygon* b2Shape_GetPolygon(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);
	
	return &shape->polygon;
}

b2ChainId b2Shape_GetParentChain(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);
	if (shape->type == b2_smoothSegmentShape)
	{
		int32_t chainIndex = shape->smoothSegment.chainIndex;
		if (chainIndex != B2_NULL_INDEX)
		{
			
			b2ChainShape* chain = world->chains + chainIndex;
			
			b2ChainId chainId = {chainIndex, shapeId.world, chain->object.revision};
			return chainId;
		}
	}

	return b2_nullChainId;
}

void b2Chain_SetFriction(b2ChainId chainId, float friction)
{
	b2World* world = b2GetWorldFromIndex(chainId.world);
	
	if (world->locked)
	{
		return;
	}

	

	b2ChainShape* chainShape = world->chains + chainId.index;
	

	int32_t count = chainShape->count;

	for (int32_t i = 0; i < count; ++i)
	{
		int32_t shapeIndex = chainShape->shapeIndices[i];
		
		b2Shape* shape = world->shapes + shapeIndex;
		shape->friction = friction;
	}
}

void b2Chain_SetRestitution(b2ChainId chainId, float restitution)
{
	b2World* world = b2GetWorldFromIndex(chainId.world);
	
	if (world->locked)
	{
		return;
	}

	

	b2ChainShape* chainShape = world->chains + chainId.index;
	

	int32_t count = chainShape->count;

	for (int32_t i = 0; i < count; ++i)
	{
		int32_t shapeIndex = chainShape->shapeIndices[i];
		
		b2Shape* shape = world->shapes + shapeIndex;
		shape->restitution = restitution;
	}
}

int32_t b2Shape_GetContactCapacity(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	
	if (world->locked)
	{
		return 0;
	}

	b2Shape* shape = b2GetShape(world, shapeId);
	b2Body* body = world->bodies + shape->bodyIndex;

	// Conservative and fast
	return body->contactCount;
}

int32_t b2Shape_GetContactData(b2ShapeId shapeId, b2ContactData* contactData, int32_t capacity)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	
	if (world->locked)
	{
		return 0;
	}

	b2Shape* shape = b2GetShape(world, shapeId);

	b2Body* body = world->bodies + shape->bodyIndex;
	int32_t contactKey = body->contactList;
	int32_t index = 0;
	while (contactKey != B2_NULL_INDEX && index < capacity)
	{
		int32_t contactIndex = contactKey >> 1;
		int32_t edgeIndex = contactKey & 1;

		b2Contact* contact = world->contacts + contactIndex;

		// Does contact involve this shape and is it touching?
		if ((contact->shapeIndexA == shapeId.index || contact->shapeIndexB == shapeId.index) &&
			(contact->flags & b2_contactTouchingFlag) != 0)
		{
			b2Shape* shapeA = world->shapes + contact->shapeIndexA;
			b2Shape* shapeB = world->shapes + contact->shapeIndexB;

			contactData[index].shapeIdA = (b2ShapeId){shapeA->object.index, shapeId.world, shapeA->object.revision};
			contactData[index].shapeIdB = (b2ShapeId){shapeB->object.index, shapeId.world, shapeB->object.revision};
			contactData[index].manifold = contact->manifold;
			index += 1;
		}

		contactKey = contact->edges[edgeIndex].nextKey;
	}

	

	return index;
}

AABB b2Shape_GetAABB(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndexLocked(shapeId.world);
	if (world == NULL)
	{
		return (AABB){0};
	}

	b2Shape* shape = b2GetShape(world, shapeId);
	return shape->aabb;
}
