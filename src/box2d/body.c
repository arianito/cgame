// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#include "body.h"

#include "aabb.h"
#include "mem/mem.h"
#include "array.h"
#include "block_allocator.h"
#include "contact.h"
#include "core.h"
#include "graph.h"
#include "island.h"
#include "joint.h"
#include "shape.h"
#include "world.h"

// needed for dll export
#include "box2d/box2d.h"
#include "box2d/event_types.h"
#include "box2d/id.h"


#include "math/vec2.h"
#include "math/rot2.h"
#include "math/tran2.h"

static void b2CreateIslandForBody(b2World* world, b2Body* body, bool isAwake)
{
	
	
	

	if (body->type == b2_staticBody)
	{
		return;
	}

	// Every new body gets a new island. Islands get merged during simulation.
	b2Island* island = (b2Island*)b2AllocObject(&world->islandPool);
	world->islands = (b2Island*)world->islandPool.memory;
	b2CreateIsland(island);
	island->world = world;

	body->islandIndex = island->object.index;
	island->headBody = body->object.index;
	island->tailBody = body->object.index;
	island->bodyCount = 1;

	if (isAwake)
	{
		island->awakeIndex = b2Array(world->awakeIslandArray).count;
		b2Array_Push(world->awakeIslandArray, island->object.index);
	}
}

static void b2RemoveBodyFromIsland(b2World* world, b2Body* body)
{
	if (body->islandIndex == B2_NULL_INDEX)
	{
		
		
		return;
	}

	b2Island* island = world->islands + body->islandIndex;

	// Fix the island's linked list of bodies
	if (body->islandPrev != B2_NULL_INDEX)
	{
		world->bodies[body->islandPrev].islandNext = body->islandNext;
	}

	if (body->islandNext != B2_NULL_INDEX)
	{
		world->bodies[body->islandNext].islandPrev = body->islandPrev;
	}

	
	island->bodyCount -= 1;
	bool islandDestroyed = false;

	if (island->headBody == body->object.index)
	{
		island->headBody = body->islandNext;

		if (island->headBody == B2_NULL_INDEX)
		{
			// Destroy empty island
			
			
			
			

			// Free the island
			b2DestroyIsland(island);
			islandDestroyed = true;
		}
	}
	else if (island->tailBody == body->object.index)
	{
		island->tailBody = body->islandPrev;
	}

	if (islandDestroyed == false)
	{
		b2WakeIsland(island);
		b2ValidateIsland(island, true);
	}

	body->islandIndex = B2_NULL_INDEX;
	body->islandPrev = B2_NULL_INDEX;
	body->islandNext = B2_NULL_INDEX;
}

static void b2DestroyBodyContacts(b2World* world, b2Body* body)
{
	// Destroy the attached contacts
	int32_t edgeKey = body->contactList;
	while (edgeKey != B2_NULL_INDEX)
	{
		int32_t contactIndex = edgeKey >> 1;
		int32_t edgeIndex = edgeKey & 1;

		b2Contact* contact = world->contacts + contactIndex;
		edgeKey = contact->edges[edgeIndex].nextKey;
		b2DestroyContact(world, contact);
	}
}

static void b2EnableBody(b2World* world, b2Body* body)
{
	// Add shapes to broad-phase
	int32_t shapeIndex = body->shapeList;
	while (shapeIndex != B2_NULL_INDEX)
	{
		b2Shape* shape = world->shapes + shapeIndex;
		shapeIndex = shape->nextShapeIndex;

		b2CreateShapeProxy(shape, &world->broadPhase, body->type, body->transform);
	}

	b2CreateIslandForBody(world, body, true);

	int32_t jointKey = body->jointList;
	while (jointKey != B2_NULL_INDEX)
	{
		int32_t jointIndex = jointKey >> 1;
		int32_t edgeIndex = jointKey & 1;
		b2Joint* joint = world->joints + jointIndex;
		
		b2Body* bodyA = world->bodies + joint->edges[0].bodyIndex;
		b2Body* bodyB = world->bodies + joint->edges[1].bodyIndex;
		if (bodyA->type == b2_dynamicBody || bodyB->type == b2_dynamicBody)
		{
			b2AddJointToGraph(world, joint);
			b2LinkJoint(world, joint);
		}
		jointKey = joint->edges[edgeIndex].nextKey;
	}
}

static void b2DisableBody(b2World* world, b2Body* body)
{
	b2DestroyBodyContacts(world, body);
	b2RemoveBodyFromIsland(world, body);

	// Remove shapes from broad-phase
	int32_t shapeIndex = body->shapeList;
	while (shapeIndex != B2_NULL_INDEX)
	{
		b2Shape* shape = world->shapes + shapeIndex;
		shapeIndex = shape->nextShapeIndex;

		b2DestroyShapeProxy(shape, &world->broadPhase);
	}

	int32_t jointKey = body->jointList;
	while (jointKey != B2_NULL_INDEX)
	{
		int32_t jointIndex = jointKey >> 1;
		int32_t edgeIndex = jointKey & 1;
		b2Joint* joint = world->joints + jointIndex;
		if (joint->colorIndex != B2_NULL_INDEX)
		{
			b2RemoveJointFromGraph(world, joint);
		}

		if (joint->islandIndex != B2_NULL_INDEX)
		{
			b2UnlinkJoint(world, joint);
		}
		jointKey = joint->edges[edgeIndex].nextKey;
	}
}

b2BodyId b2CreateBody(b2WorldId worldId, const b2BodyDef* def)
{
	b2World* world = b2GetWorldFromId(worldId);
	

	if (world->locked)
	{
		return b2_nullBodyId;
	}

	b2Body* body = (b2Body*)b2AllocObject(&world->bodyPool);
	world->bodies = (b2Body*)world->bodyPool.memory;

	
	
	
	
	
	
	
	

	body->type = def->type;
	body->transform.position = def->position;
	body->transform.rotation = rot2f(def->angle);
	body->position0 = def->position;
	body->position = def->position;
	body->angle0 = def->angle;
	body->angle = def->angle;
	body->localCenter = vec2_zero;
	body->linearVelocity = def->linearVelocity;
	body->angularVelocity = def->angularVelocity;
	body->deltaPosition = vec2_zero;
	body->deltaAngle = 0.0f;
	body->force = vec2_zero;
	body->torque = 0.0f;
	body->shapeList = B2_NULL_INDEX;
	body->chainList = B2_NULL_INDEX;
	body->jointList = B2_NULL_INDEX;
	body->jointCount = 0;
	body->contactList = B2_NULL_INDEX;
	body->contactCount = 0;
	body->mass = 0.0f;
	body->invMass = 0.0f;
	body->I = 0.0f;
	body->invI = 0.0f;
	body->minExtent = b2_huge;
	body->linearDamping = def->linearDamping;
	body->angularDamping = def->angularDamping;
	body->gravityScale = def->gravityScale;
	body->sleepTime = 0.0f;
	body->userData = def->userData;
	body->world = worldId.index;
	body->enableSleep = def->enableSleep;
	body->fixedRotation = def->fixedRotation;
	body->isEnabled = def->isEnabled;
	body->isMarked = false;
	body->enlargeAABB = false;
	body->isFast = false;
	body->isSpeedCapped = false;
	body->islandIndex = B2_NULL_INDEX;
	body->islandPrev = B2_NULL_INDEX;
	body->islandNext = B2_NULL_INDEX;

	if (body->isEnabled)
	{
		b2CreateIslandForBody(world, body, def->isAwake);
	}

	b2BodyId id = {body->object.index, worldId.index, body->object.revision};
	return id;
}

// Get a validated body from a world using an id.
b2Body* b2GetBody(b2World* world, b2BodyId id)
{
	
	b2Body* body = world->bodies + id.index;
	
	
	return body;
}

bool b2IsBodyAwake(b2World* world, b2Body* body)
{
	if (body->islandIndex != B2_NULL_INDEX)
	{
		b2Island* island = world->islands + body->islandIndex;
		return island->awakeIndex != B2_NULL_INDEX;
	}

	return false;
}

void b2WakeBody(b2World* world, b2Body* body)
{
	if (body->islandIndex != B2_NULL_INDEX)
	{
		int32_t islandIndex = body->islandIndex;
		
		b2WakeIsland(world->islands + islandIndex);
		return;
	}

	
}

void b2DestroyBodyInternal(b2World* world, b2Body* body)
{
	// User must destroy joints before destroying bodies
	

	b2DestroyBodyContacts(world, body);

	// Delete the attached shapes and their broad-phase proxies.
	int32_t shapeIndex = body->shapeList;
	while (shapeIndex != B2_NULL_INDEX)
	{
		b2Shape* shape = world->shapes + shapeIndex;
		shapeIndex = shape->nextShapeIndex;

		b2DestroyShapeProxy(shape, &world->broadPhase);
		b2FreeObject(&world->shapePool, &shape->object);
	}

	// Delete the attached chains. The associated shapes have already been deleted above.
	int32_t chainIndex = body->chainList;
	while (chainIndex != B2_NULL_INDEX)
	{
		b2ChainShape* chain = world->chains + chainIndex;
		chainIndex = chain->nextIndex;

		xxfree(chain->shapeIndices, chain->count * sizeof(int32_t));
		chain->shapeIndices = NULL;
		b2FreeObject(&world->chainPool, &chain->object);
	}

	b2RemoveBodyFromIsland(world, body);

	b2FreeObject(&world->bodyPool, &body->object);
}

void b2DestroyBody(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndexLocked(bodyId.world);
	if (world == NULL)
	{
		return;
	}

	b2Body* body = b2GetBody(world, bodyId);
	b2DestroyBodyInternal(world, body);
}

int32_t b2Body_GetContactCapacity(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndexLocked(bodyId.world);
	if (world == NULL)
	{
		return 0;
	}

	b2Body* body = b2GetBody(world, bodyId);

	// Conservative and fast
	return body->contactCount;
}

int32_t b2Body_GetContactData(b2BodyId bodyId, b2ContactData* contactData, int32_t capacity)
{
	b2World* world = b2GetWorldFromIndexLocked(bodyId.world);
	if (world == NULL)
	{
		return 0;
	}

	b2Body* body = b2GetBody(world, bodyId);

	int32_t contactKey = body->contactList;
	int32_t index = 0;
	while (contactKey != B2_NULL_INDEX && index < capacity)
	{
		int32_t contactIndex = contactKey >> 1;
		int32_t edgeIndex = contactKey & 1;

		b2Contact* contact = world->contacts + contactIndex;

		// Is contact touching?
		if (contact->flags & b2_contactTouchingFlag)
		{
			b2Shape* shapeA = world->shapes + contact->shapeIndexA;
			b2Shape* shapeB = world->shapes + contact->shapeIndexB;

			contactData[index].shapeIdA = (b2ShapeId){shapeA->object.index, bodyId.world, shapeA->object.revision};
			contactData[index].shapeIdB = (b2ShapeId){shapeB->object.index, bodyId.world, shapeB->object.revision};
			contactData[index].manifold = contact->manifold;
			index += 1;
		}

		contactKey = contact->edges[edgeIndex].nextKey;
	}

	

	return index;
}

AABB b2Body_ComputeAABB(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndexLocked(bodyId.world);
	if (world == NULL)
	{
		return (AABB){0};
	}

	b2Body* body = b2GetBody(world, bodyId);
	if (body->shapeList == B2_NULL_INDEX)
	{
		return (AABB){body->transform.position, body->transform.position};
	}

	b2Shape* shape = world->shapes + body->shapeList;
	AABB aabb = shape->aabb;
	while (shape->nextShapeIndex != B2_NULL_INDEX)
	{
		shape = world->shapes + shape->nextShapeIndex;
		aabb = aabb_union(aabb, shape->aabb);
	}

	return aabb;
}

void b2UpdateBodyMassData(b2World* world, b2Body* body)
{
	// Compute mass data from shapes. Each shape has its own density.
	body->mass = 0.0f;
	body->invMass = 0.0f;
	body->I = 0.0f;
	body->invI = 0.0f;
	body->localCenter = vec2_zero;
	body->minExtent = b2_huge;
	body->maxExtent = 0.0f;

	// Static and kinematic bodies have zero mass.
	if (body->type == b2_staticBody || body->type == b2_kinematicBody)
	{
		body->position = body->transform.position;
		return;
	}

	

	// Accumulate mass over all shapes.
	Vec2 localCenter = vec2_zero;
	int32_t shapeIndex = body->shapeList;
	while (shapeIndex != B2_NULL_INDEX)
	{
		const b2Shape* s = world->shapes + shapeIndex;
		shapeIndex = s->nextShapeIndex;

		if (s->density == 0.0f)
		{
			continue;
		}

		b2MassData massData = b2ComputeShapeMass(s);
		body->mass += massData.mass;
		localCenter = vec2_mul_add(localCenter, massData.mass, massData.center);
		body->I += massData.I;

		b2ShapeExtent extent = b2ComputeShapeExtent(s);
		body->minExtent = minf(body->minExtent, extent.minExtent);
		body->maxExtent = maxf(body->maxExtent, extent.maxExtent);
	}

	// Compute center of mass.
	if (body->mass > 0.0f)
	{
		body->invMass = 1.0f / body->mass;
		localCenter = vec2_mulfv(body->invMass, localCenter);
	}

	if (body->I > 0.0f && body->fixedRotation == false)
	{
		// Center the inertia about the center of mass.
		body->I -= body->mass * vec2_dot(localCenter, localCenter);
		
		body->invI = 1.0f / body->I;
	}
	else
	{
		body->I = 0.0f;
		body->invI = 0.0f;
	}

	// Move center of mass.
	Vec2 oldCenter = body->position;
	body->localCenter = localCenter;
	body->position = tran2_transform(body->transform, body->localCenter);

	// Update center of mass velocity.
	Vec2 deltaLinear = vec2_crossfv(body->angularVelocity, vec2_sub(body->position, oldCenter));
	body->linearVelocity = vec2_add(body->linearVelocity, deltaLinear);
}

static b2ShapeId b2CreateShape(b2BodyId bodyId, const b2ShapeDef* def, const void* geometry, b2ShapeType shapeType)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	
	if (world->locked)
	{
		return b2_nullShapeId;
	}

	b2Body* body = b2GetBody(world, bodyId);

	b2Shape* shape = (b2Shape*)b2AllocObject(&world->shapePool);
	world->shapes = (b2Shape*)world->shapePool.memory;

	
	
	

	switch (shapeType)
	{
		case b2_capsuleShape:
			shape->capsule = *(const b2Capsule*)geometry;
			break;

		case b2_circleShape:
			shape->circle = *(const b2Circle*)geometry;
			break;

		case b2_polygonShape:
			shape->polygon = *(const b2Polygon*)geometry;
			break;

		case b2_segmentShape:
			shape->segment = *(const b2Segment*)geometry;
			break;

		case b2_smoothSegmentShape:
			shape->smoothSegment = *(const b2SmoothSegment*)geometry;
			break;

		default:
			
			break;
	}

	shape->bodyIndex = body->object.index;
	shape->type = shapeType;
	shape->density = def->density;
	shape->friction = def->friction;
	shape->restitution = def->restitution;
	shape->filter = def->filter;
	shape->userData = def->userData;
	shape->isSensor = def->isSensor;
	shape->enlargedAABB = false;
	shape->enableSensorEvents = def->enableSensorEvents;
	shape->enableContactEvents = def->enableContactEvents;
	shape->enablePreSolveEvents = def->enablePreSolveEvents;
	shape->isFast = false;
	shape->proxyKey = B2_NULL_INDEX;
	shape->localCentroid = b2GetShapeCentroid(shape);
	shape->aabb = (AABB){vec2_zero, vec2_zero};
	shape->fatAABB = (AABB){vec2_zero, vec2_zero};

	if (body->isEnabled)
	{
		b2CreateShapeProxy(shape, &world->broadPhase, body->type, body->transform);
	}

	// Add to shape linked list
	shape->nextShapeIndex = body->shapeList;
	body->shapeList = shape->object.index;

	if (shape->density > 0.0f)
	{
		b2UpdateBodyMassData(world, body);
	}

	b2ShapeId id = {shape->object.index, bodyId.world, shape->object.revision};
	return id;
}

b2ShapeId b2CreateCircleShape(b2BodyId bodyId, const b2ShapeDef* def, const b2Circle* circle)
{
	return b2CreateShape(bodyId, def, circle, b2_circleShape);
}

b2ShapeId b2CreateCapsuleShape(b2BodyId bodyId, const b2ShapeDef* def, const b2Capsule* capsule)
{
	float lengthSqr = vec2_sqr_distance(capsule->point1, capsule->point2);
	if (lengthSqr <= b2_linearSlop * b2_linearSlop)
	{
		
		return b2_nullShapeId;
	}

	return b2CreateShape(bodyId, def, capsule, b2_capsuleShape);
}

b2ShapeId b2CreatePolygonShape(b2BodyId bodyId, const b2ShapeDef* def, const b2Polygon* polygon)
{
	return b2CreateShape(bodyId, def, polygon, b2_polygonShape);
}

b2ShapeId b2CreateSegmentShape(b2BodyId bodyId, const b2ShapeDef* def, const b2Segment* segment)
{
	float lengthSqr = vec2_sqr_distance(segment->point1, segment->point2);
	if (lengthSqr <= b2_linearSlop * b2_linearSlop)
	{
		
		return b2_nullShapeId;
	}

	return b2CreateShape(bodyId, def, segment, b2_segmentShape);
}

// Destroy a shape on a body. This doesn't need to be called when destroying a body.
static void b2DestroyShapeInternal(b2World* world, b2Shape* shape)
{
	int32_t shapeIndex = shape->object.index;
	
	

	b2Body* body = world->bodies + shape->bodyIndex;

	// Remove the shape from the body's singly linked list.
	int32_t* indexPtr = &body->shapeList;
	bool found = false;
	while (*indexPtr != B2_NULL_INDEX)
	{
		if (*indexPtr == shape->object.index)
		{
			*indexPtr = shape->nextShapeIndex;
			found = true;
			break;
		}

		indexPtr = &(world->shapes[*indexPtr].nextShapeIndex);
	}

	
	if (found == false)
	{
		return;
	}

	const float density = shape->density;

	// Destroy any contacts associated with the shape
	int32_t contactKey = body->contactList;
	while (contactKey != B2_NULL_INDEX)
	{
		int32_t contactIndex = contactKey >> 1;
		int32_t edgeIndex = contactKey & 1;

		b2Contact* contact = world->contacts + contactIndex;
		contactKey = contact->edges[edgeIndex].nextKey;

		if (contact->shapeIndexA == shapeIndex || contact->shapeIndexB == shapeIndex)
		{
			b2DestroyContact(world, contact);
		}
	}

	if (body->isEnabled)
	{
		b2DestroyShapeProxy(shape, &world->broadPhase);
	}

	b2FreeObject(&world->shapePool, &shape->object);

	// Reset the mass data
	if (density > 0.0f)
	{
		b2UpdateBodyMassData(world, body);
	}
}

// Destroy a shape on a body. This doesn't need to be called when destroying a body.
void b2DestroyShape(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	
	if (world->locked)
	{
		return;
	}

	b2Shape* shape = b2GetShape(world, shapeId);

	b2DestroyShapeInternal(world, shape);
}

b2ChainId b2CreateChain(b2BodyId bodyId, const b2ChainDef* def)
{
	
	
	

	b2World* world = b2GetWorldFromIndex(bodyId.world);
	
	if (world->locked)
	{
		return b2_nullChainId;
	}

	b2Body* body = b2GetBody(world, bodyId);

	b2ChainShape* chainShape = (b2ChainShape*)b2AllocObject(&world->chainPool);
	world->chains = (b2ChainShape*)world->chainPool.memory;

	int32_t chainIndex = chainShape->object.index;
	chainShape->bodyIndex = bodyId.index;
	chainShape->nextIndex = body->chainList;
	body->chainList = chainShape->object.index;

	b2ShapeDef shapeDef = b2_defaultShapeDef;
	shapeDef.userData = def->userData;
	shapeDef.restitution = def->restitution;
	shapeDef.friction = def->friction;
	shapeDef.filter = def->filter;
	shapeDef.enableContactEvents = false;
	shapeDef.enableSensorEvents = false;

	int32_t n = def->count;
	const Vec2* points = def->points;

	if (def->loop)
	{
		chainShape->count = n;
		chainShape->shapeIndices = xxmalloc(n * sizeof(int32_t));

		b2SmoothSegment smoothSegment;

		int32_t prevIndex = n - 1;
		for (int32_t i = 0; i < n - 2; ++i)
		{
			smoothSegment.ghost1 = points[prevIndex];
			smoothSegment.segment.point1 = points[i];
			smoothSegment.segment.point2 = points[i + 1];
			smoothSegment.ghost2 = points[i + 2];
			smoothSegment.chainIndex = chainIndex;
			prevIndex = i;

			b2ShapeId shapeId = b2CreateShape(bodyId, &shapeDef, &smoothSegment, b2_smoothSegmentShape);
			chainShape->shapeIndices[i] = shapeId.index;
		}

		{
			smoothSegment.ghost1 = points[n - 3];
			smoothSegment.segment.point1 = points[n - 2];
			smoothSegment.segment.point2 = points[n - 1];
			smoothSegment.ghost2 = points[0];
			smoothSegment.chainIndex = chainIndex;
			b2ShapeId shapeId = b2CreateShape(bodyId, &shapeDef, &smoothSegment, b2_smoothSegmentShape);
			chainShape->shapeIndices[n - 2] = shapeId.index;
		}

		{
			smoothSegment.ghost1 = points[n - 2];
			smoothSegment.segment.point1 = points[n - 1];
			smoothSegment.segment.point2 = points[0];
			smoothSegment.ghost2 = points[1];
			smoothSegment.chainIndex = chainIndex;
			b2ShapeId shapeId = b2CreateShape(bodyId, &shapeDef, &smoothSegment, b2_smoothSegmentShape);
			chainShape->shapeIndices[n - 1] = shapeId.index;
		}
	}
	else
	{
		chainShape->count = n - 3;
		chainShape->shapeIndices = xxmalloc(n * sizeof(int32_t));

		b2SmoothSegment smoothSegment;

		for (int32_t i = 0; i < n - 3; ++i)
		{
			smoothSegment.ghost1 = points[i];
			smoothSegment.segment.point1 = points[i + 1];
			smoothSegment.segment.point2 = points[i + 2];
			smoothSegment.ghost2 = points[i + 3];
			smoothSegment.chainIndex = chainIndex;

			b2ShapeId shapeId = b2CreateShape(bodyId, &shapeDef, &smoothSegment, b2_smoothSegmentShape);
			chainShape->shapeIndices[i] = shapeId.index;
		}
	}

	b2ChainId id = {chainShape->object.index, bodyId.world, chainShape->object.revision};
	return id;
}

void b2DestroyChain(b2ChainId chainId)
{
	b2World* world = b2GetWorldFromIndex(chainId.world);
	
	if (world->locked)
	{
		return;
	}

	

	b2ChainShape* chain = world->chains + chainId.index;
	

	// Remove the chain from the body's singly linked list.
	b2Body* body = world->bodies + chain->bodyIndex;
	int32_t* indexPtr = &body->chainList;
	bool found = false;
	while (*indexPtr != B2_NULL_INDEX)
	{
		if (*indexPtr == chain->object.index)
		{
			*indexPtr = chain->nextIndex;
			found = true;
			break;
		}

		indexPtr = &(world->chains[*indexPtr].nextIndex);
	}

	
	if (found == false)
	{
		return;
	}

	int32_t count = chain->count;
	for (int32_t i = 0; i < count; ++i)
	{
		int32_t shapeIndex = chain->shapeIndices[i];
		
		b2Shape* shape = world->shapes + shapeIndex;
		b2DestroyShapeInternal(world, shape);
	}

	xxfree(chain->shapeIndices, count * sizeof(int32_t));
	b2FreeObject(&world->chainPool, &chain->object);
}

Vec2 b2Body_GetPosition(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return body->transform.position;
}

float b2Body_GetAngle(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return body->angle;
}

Tran2 b2Body_GetTransform(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return body->transform;
}

Vec2 b2Body_GetLocalPoint(b2BodyId bodyId, Vec2 globalPoint)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return tran2_untransform(body->transform, globalPoint);
}

Vec2 b2Body_GetWorldPoint(b2BodyId bodyId, Vec2 localPoint)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return tran2_transform(body->transform, localPoint);
}

Vec2 b2Body_GetLocalVector(b2BodyId bodyId, Vec2 globalVector)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return rot2_unrotate(body->transform.rotation, globalVector);
}

Vec2 b2Body_GetWorldVector(b2BodyId bodyId, Vec2 localVector)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return rot2_rotate(body->transform.rotation, localVector);
}

void b2Body_SetTransform(b2BodyId bodyId, Vec2 position, float angle)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	

	b2Body* body = b2GetBody(world, bodyId);

	body->transform.position = position;
	body->transform.rotation = rot2f(angle);

	body->position = tran2_transform(body->transform, body->localCenter);
	body->angle = angle;

	body->position0 = body->position;
	body->angle0 = body->angle;

	b2BroadPhase* broadPhase = &world->broadPhase;

	const Vec2 aabbMargin = {b2_aabbMargin, b2_aabbMargin};
	int32_t shapeIndex = body->shapeList;
	while (shapeIndex != B2_NULL_INDEX)
	{
		b2Shape* shape = world->shapes + shapeIndex;
		shape->aabb = b2ComputeShapeAABB(shape, body->transform);

		if (aabb_contains(shape->fatAABB, shape->aabb) == false)
		{
			shape->fatAABB.min = vec2_sub(shape->aabb.min, aabbMargin);
			shape->fatAABB.max = vec2_add(shape->aabb.max, aabbMargin);
			b2BroadPhase_MoveProxy(broadPhase, shape->proxyKey, shape->fatAABB);
		}

		shapeIndex = shape->nextShapeIndex;
	}
}

Vec2 b2Body_GetLinearVelocity(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return body->linearVelocity;
}

float b2Body_GetAngularVelocity(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return body->angularVelocity;
}

void b2Body_SetLinearVelocity(b2BodyId bodyId, Vec2 linearVelocity)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	if (body->type == b2_staticBody || body->isEnabled == false)
	{
		return;
	}

	body->linearVelocity = linearVelocity;
}

void b2Body_SetAngularVelocity(b2BodyId bodyId, float angularVelocity)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	if (body->type == b2_staticBody || body->isEnabled == false)
	{
		return;
	}

	body->angularVelocity = angularVelocity;

	if (angularVelocity != 0.0f)
	{
		b2WakeBody(world, body);
	}
}

void b2Body_ApplyForce(b2BodyId bodyId, Vec2 force, Vec2 point, bool wake)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	if (body->type == b2_staticBody || body->isEnabled == false)
	{
		return;
	}

	if (wake)
	{
		b2WakeBody(world, body);
	}

	if (b2IsBodyAwake(world, body))
	{
		body->force = vec2_add(body->force, force);
		body->torque += vec2_cross(vec2_sub(point, body->position), force);
	}
}

void b2Body_ApplyForceToCenter(b2BodyId bodyId, Vec2 force, bool wake)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	if (body->type == b2_staticBody || body->isEnabled == false)
	{
		return;
	}

	if (wake)
	{
		b2WakeBody(world, body);
	}

	if (b2IsBodyAwake(world, body))
	{
		body->force = vec2_add(body->force, force);
	}
}

void b2Body_ApplyTorque(b2BodyId bodyId, float torque, bool wake)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	if (body->type == b2_staticBody || body->isEnabled == false)
	{
		return;
	}

	if (wake)
	{
		b2WakeBody(world, body);
	}

	if (b2IsBodyAwake(world, body))
	{
		body->torque += torque;
	}
}

void b2Body_ApplyLinearImpulse(b2BodyId bodyId, Vec2 impulse, Vec2 point, bool wake)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	if (body->type == b2_staticBody || body->isEnabled == false)
	{
		return;
	}

	if (wake)
	{
		b2WakeBody(world, body);
	}

	if (b2IsBodyAwake(world, body))
	{
		body->linearVelocity = vec2_mul_add(body->linearVelocity, body->invMass, impulse);
		body->angularVelocity += body->invI * vec2_cross(vec2_sub(point, body->position), impulse);
	}
}

void b2Body_ApplyLinearImpulseToCenter(b2BodyId bodyId, Vec2 impulse, bool wake)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	if (body->type == b2_staticBody || body->isEnabled == false)
	{
		return;
	}

	if (wake)
	{
		b2WakeBody(world, body);
	}

	if (b2IsBodyAwake(world, body))
	{
		body->linearVelocity = vec2_mul_add(body->linearVelocity, body->invMass, impulse);
	}
}

void b2Body_ApplyAngularImpulse(b2BodyId bodyId, float impulse, bool wake)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	if (body->type == b2_staticBody || body->isEnabled == false)
	{
		return;
	}

	if (wake)
	{
		b2WakeBody(world, body);
	}

	if (b2IsBodyAwake(world, body))
	{
		body->angularVelocity += impulse;
	}
}

b2BodyType b2Body_GetType(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return body->type;
}

void b2Body_SetType(b2BodyId bodyId, b2BodyType type)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	if (body->type == type)
	{
		return;
	}

	if (body->isEnabled == true)
	{
		b2DisableBody(world, body);

		body->type = type;

		b2EnableBody(world, body);
	}
	else
	{
		body->type = type;
	}

	// Body type affects the mass
	b2UpdateBodyMassData(world, body);
}

void* b2Body_GetUserData(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return body->userData;
}

float b2Body_GetMass(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return body->mass;
}

float b2Body_GetInertiaTensor(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return body->I;
}

Vec2 b2Body_GetLocalCenterOfMass(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return body->localCenter;
}

Vec2 b2Body_GetWorldCenterOfMass(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return body->position;
}

void b2Body_SetMassData(b2BodyId bodyId, b2MassData massData)
{
	
	
	

	b2World* world = b2GetWorldFromIndexLocked(bodyId.world);
	if (world == NULL)
	{
		return;
	}

	b2Body* body = b2GetBody(world, bodyId);
	body->mass = massData.mass;
	body->I = massData.I;
	body->localCenter = massData.center;

	Vec2 p = tran2_transform(body->transform, massData.center);
	body->position = p;
	body->position0 = p;

	body->invMass = body->mass > 0.0f ? 1.0f / body->mass : 0.0f;
	body->invI = body->I > 0.0f ? 1.0f / body->I : 0.0f;
}

b2MassData b2Body_GetMassData(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	b2MassData massData = {body->mass, body->localCenter, body->I};
	return massData;
}

void b2Body_SetLinearDamping(b2BodyId bodyId, float linearDamping)
{
	

	b2World* world = b2GetWorldFromIndexLocked(bodyId.world);
	if (world == NULL)
	{
		return;
	}

	b2Body* body = b2GetBody(world, bodyId);
	body->linearDamping = linearDamping;
}

float b2Body_GetLinearDamping(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return body->linearDamping;
}

void b2Body_SetAngularDamping(b2BodyId bodyId, float angularDamping)
{
	

	b2World* world = b2GetWorldFromIndexLocked(bodyId.world);
	if (world == NULL)
	{
		return;
	}

	b2Body* body = b2GetBody(world, bodyId);
	body->angularDamping = angularDamping;
}

float b2Body_GetAngularDamping(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return body->angularDamping;
}

void b2Body_SetGravityScale(b2BodyId bodyId, float gravityScale)
{
	

	b2World* world = b2GetWorldFromIndexLocked(bodyId.world);
	if (world == NULL)
	{
		return;
	}

	b2Body* body = b2GetBody(world, bodyId);
	body->gravityScale = gravityScale;
}

float b2Body_GetGravityScale(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return body->gravityScale;
}

bool b2Body_IsAwake(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return b2IsBodyAwake(world, body);
}

void b2Body_Wake(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndexLocked(bodyId.world);
	if (world == NULL)
	{
		return;
	}

	b2Body* body = b2GetBody(world, bodyId);
	if (body->type == b2_staticBody)
	{
		return;
	}

	b2WakeBody(world, body);
}

bool b2Body_IsEnabled(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	return body->isEnabled;
}

void b2Body_Disable(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndexLocked(bodyId.world);
	if (world == NULL)
	{
		return;
	}

	b2Body* body = b2GetBody(world, bodyId);
	if (body->isEnabled == true)
	{
		b2DisableBody(world, body);
		body->isEnabled = false;
	}
}

void b2Body_Enable(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);
	if (body->isEnabled == false)
	{
		b2EnableBody(world, body);
		body->isEnabled = true;
	}
}

b2ShapeId b2Body_GetFirstShape(b2BodyId bodyId)
{
	b2World* world = b2GetWorldFromIndex(bodyId.world);
	b2Body* body = b2GetBody(world, bodyId);

	if (body->shapeList == B2_NULL_INDEX)
	{
		return b2_nullShapeId;
	}

	b2Shape* shape = world->shapes + body->shapeList;
	b2ShapeId id = {shape->object.index, bodyId.world, shape->object.revision};
	return id;
}

b2ShapeId b2Body_GetNextShape(b2ShapeId shapeId)
{
	b2World* world = b2GetWorldFromIndex(shapeId.world);
	b2Shape* shape = b2GetShape(world, shapeId);

	if (shape->nextShapeIndex == B2_NULL_INDEX)
	{
		return b2_nullShapeId;
	}

	shape = world->shapes + shape->nextShapeIndex;
	b2ShapeId id = {shape->object.index, shapeId.world, shape->object.revision};
	return id;
}

bool b2ShouldBodiesCollide(b2World* world, b2Body* bodyA, b2Body* bodyB)
{
	int32_t jointKey;
	int32_t otherBodyIndex;
	if (bodyA->jointCount < bodyB->jointCount)
	{
		jointKey = bodyA->jointList;
		otherBodyIndex = bodyB->object.index;
	}
	else
	{
		jointKey = bodyB->jointList;
		otherBodyIndex = bodyA->object.index;
	}

	while (jointKey != B2_NULL_INDEX)
	{
		int32_t jointIndex = jointKey >> 1;
		int32_t edgeIndex = jointKey & 1;
		int32_t otherEdgeIndex = edgeIndex ^ 1;

		b2Joint* joint = world->joints + jointIndex;
		if (joint->collideConnected == false && joint->edges[otherEdgeIndex].bodyIndex == otherBodyIndex)
		{
			return false;
		}

		jointKey = joint->edges[edgeIndex].nextKey;
	}

	return true;
}

#if 0
void b2Body_Dump(b2Body* b)
{
	int32_t bodyIndex = body->islandIndex;

	// %.9g is sufficient to save and load the same value using text
	// FLT_DECIMAL_DIG == 9

	b2Dump("{\n");
	b2Dump("  b2BodyDef bd;\n");
	b2Dump("  bd.type = b2BodyType(%d);\n", body->type);
	b2Dump("  bd.position.Set(%.9g, %.9g);\n", m_xf.position.x, m_xf.position.y);
	b2Dump("  bd.angle = %.9g;\n", m_sweep.a);
	b2Dump("  bd.linearVelocity.Set(%.9g, %.9g);\n", m_linearVelocity.x, m_linearVelocity.y);
	b2Dump("  bd.angularVelocity = %.9g;\n", m_angularVelocity);
	b2Dump("  bd.linearDamping = %.9g;\n", m_linearDamping);
	b2Dump("  bd.angularDamping = %.9g;\n", m_angularDamping);
	b2Dump("  bd.allowSleep = bool(%d);\n", m_flags & e_autoSleepFlag);
	b2Dump("  bd.awake = bool(%d);\n", m_flags & e_awakeFlag);
	b2Dump("  bd.fixedRotation = bool(%d);\n", m_flags & e_fixedRotationFlag);
	b2Dump("  bd.bullet = bool(%d);\n", m_flags & e_bulletFlag);
	b2Dump("  bd.enabled = bool(%d);\n", m_flags & e_enabledFlag);
	b2Dump("  bd.gravityScale = %.9g;\n", m_gravityScale);
	b2Dump("  bodies[%d] = m_world->CreateBody(&bd);\n", m_islandIndex);
	b2Dump("\n");
	for (b2Shape* shape = m_shapeList; shape; shape = shape->m_next)
	{
		b2Dump("  {\n");
		shape->Dump(bodyIndex);
		b2Dump("  }\n");
	}
	b2Dump("}\n");
}
#endif
