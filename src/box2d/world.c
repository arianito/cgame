// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#define _CRT_SECURE_NO_WARNINGS

#include "world.h"

#include "aabb.h"
#include "mem/mem.h"
#include "arena_allocator.h"
#include "array.h"
#include "bitset.h"
#include "bitset.h"
#include "block_allocator.h"
#include "body.h"
#include "broad_phase.h"
#include "contact.h"
#include "core.h"
#include "graph.h"
#include "island.h"
#include "joint.h"
#include "pool.h"
#include "shape.h"
#include "solver_data.h"

// needed for dll export
#include "box2d.h"
#include "constants.h"
#include "debug_draw.h"
#include "distance.h"
#include "event_types.h"
#include "timer.h"

#include <float.h>
#include <stdio.h>
#include <string.h>

b2World b2_worlds[b2_maxWorlds];

b2World* b2GetWorldFromId(b2WorldId id)
{
	
	b2World* world = b2_worlds + id.index;
	
	return world;
}

b2World* b2GetWorldFromIndex(int16_t index)
{
	
	b2World* world = b2_worlds + index;
	
	return world;
}

b2World* b2GetWorldFromIndexLocked(int16_t index)
{
	
	b2World* world = b2_worlds + index;
	
	if (world->locked)
	{
		
		return NULL;
	}

	return world;
}

static void* b2DefaultAddTaskFcn(b2TaskCallback* task, int32_t count, int32_t minRange, void* taskContext, void* userContext)
{
	B2_MAYBE_UNUSED(minRange);
	B2_MAYBE_UNUSED(userContext);
	task(0, count, 0, taskContext);
	return NULL;
}

static void b2DefaultFinishTaskFcn(void* userTask, void* userContext)
{
	B2_MAYBE_UNUSED(userTask);
	B2_MAYBE_UNUSED(userContext);
}

b2WorldId b2CreateWorld(const b2WorldDef* def)
{
	b2WorldId id = b2_nullWorldId;
	for (int16_t i = 0; i < b2_maxWorlds; ++i)
	{
		if (b2_worlds[i].blockAllocator == NULL)
		{
			id.index = i;
			break;
		}
	}

	if (id.index == b2_nullWorldId.index)
	{
		return id;
	}

	b2InitializeContactRegisters();

	b2World empty = {0};
	b2World* world = b2_worlds + id.index;
	*world = empty;

	world->index = id.index;

	world->blockAllocator = b2CreateBlockAllocator();
	world->stackAllocator = b2CreateStackAllocator(def->arenaAllocatorCapacity);

	b2CreateBroadPhase(&world->broadPhase);
	b2CreateGraph(&world->graph, def->bodyCapacity, def->contactCapacity, def->jointCapacity);

	// pools
	world->bodyPool = b2CreatePool(sizeof(b2Body), maxf(def->bodyCapacity, 1));
	world->bodies = (b2Body*)world->bodyPool.memory;

	world->shapePool = b2CreatePool(sizeof(b2Shape), maxf(def->shapeCapacity, 1));
	world->shapes = (b2Shape*)world->shapePool.memory;

	world->chainPool = b2CreatePool(sizeof(b2ChainShape), 4);
	world->chains = (b2ChainShape*)world->chainPool.memory;

	world->contactPool = b2CreatePool(sizeof(b2Contact), maxf(def->contactCapacity, 1));
	world->contacts = (b2Contact*)world->contactPool.memory;

	world->jointPool = b2CreatePool(sizeof(b2Joint), maxf(def->jointCapacity, 1));
	world->joints = (b2Joint*)world->jointPool.memory;

	world->islandPool = b2CreatePool(sizeof(b2Island), maxf(def->bodyCapacity, 1));
	world->islands = (b2Island*)world->islandPool.memory;

	world->awakeIslandArray = b2CreateArray(sizeof(int32_t), maxf(def->bodyCapacity, 1));

	world->awakeContactArray = b2CreateArray(sizeof(int32_t), maxf(def->contactCapacity, 1));
	world->contactAwakeIndexArray = b2CreateArray(sizeof(int32_t), world->contactPool.capacity);

	world->sensorBeginEventArray = b2CreateArray(sizeof(b2SensorBeginTouchEvent), 4);
	world->sensorEndEventArray = b2CreateArray(sizeof(b2SensorEndTouchEvent), 4);

	world->contactBeginArray = b2CreateArray(sizeof(b2ContactBeginTouchEvent), 4);
	world->contactEndArray = b2CreateArray(sizeof(b2ContactEndTouchEvent), 4);

	world->stepId = 0;
	world->activeTaskCount = 0;
	world->taskCount = 0;

	// Globals start at 0. It should be fine for this to roll over.
	world->revision += 1;

	world->gravity = def->gravity;
	world->restitutionThreshold = def->restitutionThreshold;
	world->contactPushoutVelocity = def->contactPushoutVelocity;
	world->contactHertz = def->contactHertz;
	world->contactDampingRatio = def->contactDampingRatio;
	world->inv_dt0 = 0.0f;
	world->enableSleep = true;
	world->locked = false;
	world->enableWarmStarting = true;
	world->enableContinuous = true;
	world->profile = b2_emptyProfile;
	world->userTreeTask = NULL;
	world->splitIslandIndex = B2_NULL_INDEX;

	id.revision = world->revision;

	if (def->workerCount > 0 && def->enqueueTask != NULL && def->finishTask != NULL)
	{
		world->workerCount = minf(def->workerCount, b2_maxWorkers);
		world->enqueueTaskFcn = def->enqueueTask;
		world->finishTaskFcn = def->finishTask;
		world->userTaskContext = def->userTaskContext;
	}
	else
	{
		world->workerCount = 1;
		world->enqueueTaskFcn = b2DefaultAddTaskFcn;
		world->finishTaskFcn = b2DefaultFinishTaskFcn;
		world->userTaskContext = NULL;
	}

	world->taskContextArray = b2CreateArray(sizeof(b2TaskContext), world->workerCount);
	for (uint32_t i = 0; i < world->workerCount; ++i)
	{
		world->taskContextArray[i].contactStateBitSet = b2CreateBitSet(def->contactCapacity);
		world->taskContextArray[i].awakeContactBitSet = b2CreateBitSet(def->contactCapacity);
		world->taskContextArray[i].shapeBitSet = b2CreateBitSet(def->shapeCapacity);
		world->taskContextArray[i].awakeIslandBitSet = b2CreateBitSet(256);
	}

	return id;
}

void b2DestroyWorld(b2WorldId id)
{
	b2World* world = b2GetWorldFromId(id);

	for (uint32_t i = 0; i < world->workerCount; ++i)
	{
		b2DestroyBitSet(&world->taskContextArray[i].contactStateBitSet);
		b2DestroyBitSet(&world->taskContextArray[i].awakeContactBitSet);
		b2DestroyBitSet(&world->taskContextArray[i].shapeBitSet);
		b2DestroyBitSet(&world->taskContextArray[i].awakeIslandBitSet);
	}

	b2DestroyArray(world->taskContextArray, sizeof(b2TaskContext));
	b2DestroyArray(world->awakeContactArray, sizeof(int32_t));
	b2DestroyArray(world->awakeIslandArray, sizeof(int32_t));
	b2DestroyArray(world->contactAwakeIndexArray, sizeof(int32_t));

	b2DestroyArray(world->sensorBeginEventArray, sizeof(b2SensorBeginTouchEvent));
	b2DestroyArray(world->sensorEndEventArray, sizeof(b2SensorEndTouchEvent));

	b2DestroyArray(world->contactBeginArray, sizeof(b2ContactBeginTouchEvent));
	b2DestroyArray(world->contactEndArray, sizeof(b2ContactEndTouchEvent));

	b2DestroyPool(&world->islandPool);
	b2DestroyPool(&world->jointPool);
	b2DestroyPool(&world->contactPool);
	b2DestroyPool(&world->shapePool);

	int32_t chainCapacity = world->chainPool.capacity;
	for (int32_t i = 0; i < chainCapacity; ++i)
	{
		b2ChainShape* chain = world->chains + i;
		if (b2ObjectValid(&chain->object))
		{
			xxfree(chain->shapeIndices, chain->count * sizeof(int32_t));
		}
	}

	b2DestroyPool(&world->chainPool);
	b2DestroyPool(&world->bodyPool);

	b2DestroyGraph(&world->graph);
	b2DestroyBroadPhase(&world->broadPhase);

	b2DestroyBlockAllocator(world->blockAllocator);
	b2DestroyStackAllocator(world->stackAllocator);

	*world = (b2World){0};
}

static void b2CollideTask(int32_t startIndex, int32_t endIndex, uint32_t threadIndex, void* context)
{
	b2TracyCZoneNC(collide_task, "Collide Task", b2_colorDodgerBlue1, true);

	b2World* world = context;
	
	b2TaskContext* taskContext = world->taskContextArray + threadIndex;
	b2Shape* shapes = world->shapes;
	b2Body* bodies = world->bodies;
	b2Contact* contacts = world->contacts;
	int32_t awakeCount = b2Array(world->awakeContactArray).count;
	int32_t* awakeContactArray = world->awakeContactArray;
	int32_t* contactAwakeIndexArray = world->contactAwakeIndexArray;

	B2_MAYBE_UNUSED(awakeCount);
	
	

	for (int32_t awakeIndex = startIndex; awakeIndex < endIndex; ++awakeIndex)
	{
		int32_t contactIndex = awakeContactArray[awakeIndex];
		if (contactIndex == B2_NULL_INDEX)
		{
			// Contact was destroyed
			continue;
		}

		
		b2Contact* contact = contacts + contactIndex;

		
		

		// Reset contact awake index. Contacts must be added to the awake contact array
		// each time step in the island solver.
		contactAwakeIndexArray[contactIndex] = B2_NULL_INDEX;

		b2Shape* shapeA = shapes + contact->shapeIndexA;
		b2Shape* shapeB = shapes + contact->shapeIndexB;

		// Do proxies still overlap?
		bool overlap = aabb_overlaps(shapeA->fatAABB, shapeB->fatAABB);
		if (overlap == false)
		{
			contact->flags |= b2_contactDisjoint;
			b2SetBit(&taskContext->contactStateBitSet, awakeIndex);
		}
		else
		{
			bool wasTouching = (contact->flags & b2_contactTouchingFlag);
			

			// Update contact respecting shape/body order (A,B)
			b2Body* bodyA = bodies + shapeA->bodyIndex;
			b2Body* bodyB = bodies + shapeB->bodyIndex;
			b2UpdateContact(world, contact, shapeA, bodyA, shapeB, bodyB);

			bool touching = (contact->flags & b2_contactTouchingFlag) != 0;

			// State changes that affect island connectivity
			if (touching == true && wasTouching == false)
			{
				contact->flags |= b2_contactStartedTouching;
				b2SetBit(&taskContext->contactStateBitSet, awakeIndex);
			}
			else if (touching == false && wasTouching == true)
			{
				contact->flags |= b2_contactStoppedTouching;
				b2SetBit(&taskContext->contactStateBitSet, awakeIndex);
			}
		}
	}

	b2TracyCZoneEnd(collide_task);
}

static void b2UpdateTreesTask(int32_t startIndex, int32_t endIndex, uint32_t threadIndex, void* context)
{
	B2_MAYBE_UNUSED(startIndex);
	B2_MAYBE_UNUSED(endIndex);
	B2_MAYBE_UNUSED(threadIndex);

	b2TracyCZoneNC(tree_task, "Rebuild Trees", b2_colorSnow1, true);

	b2World* world = context;
	b2BroadPhase_RebuildTrees(&world->broadPhase);

	b2TracyCZoneEnd(tree_task);
}

// Narrow-phase collision
static void b2Collide(b2World* world)
{
	

	b2TracyCZoneNC(collide, "Collide", b2_colorDarkOrchid, true);

	// Tasks that can be done in parallel with the narrow-phase
	// - rebuild the collision tree for dynamic and kinematic bodies to keep their query performance good
	world->userTreeTask = world->enqueueTaskFcn(&b2UpdateTreesTask, 1, 1, world, world->userTaskContext);
	world->taskCount += 1;
	world->activeTaskCount += world->userTreeTask == NULL ? 0 : 1;

	int32_t awakeContactCount = b2Array(world->awakeContactArray).count;

	if (awakeContactCount == 0)
	{
		b2TracyCZoneEnd(collide);
		return;
	}

	for (uint32_t i = 0; i < world->workerCount; ++i)
	{
		b2SetBitCountAndClear(&world->taskContextArray[i].contactStateBitSet, awakeContactCount);
	}

	// Task should take at least 40us on a 4GHz CPU (10K cycles)
	int32_t minRange = 64;
	void* userCollideTask = world->enqueueTaskFcn(&b2CollideTask, awakeContactCount, minRange, world, world->userTaskContext);
	world->taskCount += 1;
	if (userCollideTask != NULL)
	{
		world->finishTaskFcn(userCollideTask, world->userTaskContext);
	}

	// Serially update contact state
	b2TracyCZoneNC(contact_state, "Contact State", b2_colorCoral, true);

	// Bitwise OR all contact bits
	b2BitSet* bitSet = &world->taskContextArray[0].contactStateBitSet;
	for (uint32_t i = 1; i < world->workerCount; ++i)
	{
		b2InPlaceUnion(bitSet, &world->taskContextArray[i].contactStateBitSet);
	}

	// Prepare to capture events
	b2Array_Clear(world->sensorBeginEventArray);
	b2Array_Clear(world->sensorEndEventArray);
	b2Array_Clear(world->contactBeginArray);
	b2Array_Clear(world->contactEndArray);

	const b2Shape* shapes = world->shapes;
	int16_t worldIndex = world->index;

	// Process contact state changes. Iterate over set bits
	uint64_t word;
	for (uint32_t k = 0; k < bitSet->blockCount; ++k)
	{
		word = bitSet->bits[k];
		while (word != 0)
		{
			uint32_t ctz = b2CTZ(word);
			uint32_t awakeIndex = 64 * k + ctz;
			

			int32_t contactIndex = world->awakeContactArray[awakeIndex];
			

			b2Contact* contact = world->contacts + contactIndex;
			const b2Shape* shapeA = shapes + contact->shapeIndexA;
			const b2Shape* shapeB = shapes + contact->shapeIndexB;
			b2ShapeId shapeIdA = {shapeA->object.index, worldIndex, shapeA->object.revision};
			b2ShapeId shapeIdB = {shapeB->object.index, worldIndex, shapeB->object.revision};
			uint32_t flags = contact->flags;

			if (flags & b2_contactDisjoint)
			{
				// Was touching?
				if ((flags & b2_contactTouchingFlag) != 0 && (flags & b2_contactEnableContactEvents) != 0)
				{
					b2ContactEndTouchEvent event = {shapeIdA, shapeIdB};
					b2Array_Push(world->contactEndArray, event);
				}

				// Bounding boxes no longer overlap
				b2DestroyContact(world, contact);
			}
			else if (flags & b2_contactStartedTouching)
			{
				
				if ((flags & b2_contactSensorFlag) != 0 && (flags & b2_contactEnableSensorEvents) != 0)
				{
					if (shapeA->isSensor)
					{
						b2SensorBeginTouchEvent event = {shapeIdA, shapeIdB};
						b2Array_Push(world->sensorBeginEventArray, event);
					}

					if (shapeB->isSensor)
					{
						b2SensorBeginTouchEvent event = {shapeIdB, shapeIdA};
						b2Array_Push(world->sensorBeginEventArray, event);
					}
				}
				else
				{
					if (flags & b2_contactEnableContactEvents)
					{
						b2ContactBeginTouchEvent event = {shapeIdA, shapeIdB, contact->manifold};
						b2Array_Push(world->contactBeginArray, event);
					}

					b2LinkContact(world, contact);
					b2AddContactToGraph(world, contact);
				}

				contact->flags &= ~b2_contactStartedTouching;
			}
			else
			{
				
				if ((flags & b2_contactSensorFlag) != 0 && (flags & b2_contactEnableSensorEvents) != 0)
				{
					if (shapeA->isSensor)
					{
						b2SensorEndTouchEvent event = {shapeIdA, shapeIdB};
						b2Array_Push(world->sensorEndEventArray, event);
					}

					if (shapeB->isSensor)
					{
						b2SensorEndTouchEvent event = {shapeIdB, shapeIdA};
						b2Array_Push(world->sensorEndEventArray, event);
					}
				}
				else
				{
					if (contact->flags & b2_contactEnableContactEvents)
					{
						b2ContactEndTouchEvent event = {shapeIdA, shapeIdB};
						b2Array_Push(world->contactEndArray, event);
					}

					b2UnlinkContact(world, contact);
					b2RemoveContactFromGraph(world, contact);
				}

				contact->flags &= ~b2_contactStoppedTouching;
			}

			// Clear the smallest set bit
			word = word & (word - 1);
		}
	}

	b2TracyCZoneEnd(contact_state);

	b2TracyCZoneEnd(collide);
}

void b2World_Step(b2WorldId worldId, float timeStep, int32_t velocityIterations, int32_t relaxIterations)
{
	if (timeStep == 0.0f)
	{
		// TODO_ERIN would be useful to still process collision while paused
		return;
	}

	b2TracyCZoneNC(world_step, "Step", b2_colorChartreuse, true);

	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return;
	}

	world->profile = b2_emptyProfile;
	world->activeTaskCount = 0;
	world->taskCount = 0;

	b2Timer stepTimer = b2CreateTimer();

	// Update collision pairs and create contacts
	{
		b2Timer timer = b2CreateTimer();
		b2UpdateBroadPhasePairs(world);
		world->profile.pairs = b2GetMilliseconds(&timer);
	}

	// TODO_ERIN atomic
	world->locked = true;

	b2StepContext context = {0};
	context.dt = timeStep;
	context.velocityIterations = velocityIterations;
	context.relaxIterations = relaxIterations;
	if (timeStep > 0.0f)
	{
		context.inv_dt = 1.0f / timeStep;
	}
	else
	{
		context.inv_dt = 0.0f;
	}

	context.dtRatio = world->inv_dt0 * timeStep;
	context.restitutionThreshold = world->restitutionThreshold;
	context.enableWarmStarting = world->enableWarmStarting;
	context.bodies = world->bodies;
	context.bodyCapacity = world->bodyPool.capacity;

	// Update contacts
	{
		b2Timer timer = b2CreateTimer();
		b2Collide(world);
		world->profile.collide = b2GetMilliseconds(&timer);
	}

	// Integrate velocities, solve velocity constraints, and integrate positions.
	if (context.dt > 0.0f)
	{
		b2Timer timer = b2CreateTimer();
		b2Solve(world, &context);
		world->profile.solve = b2GetMilliseconds(&timer);
	}

	if (context.dt > 0.0f)
	{
		world->inv_dt0 = context.inv_dt;
	}

	world->locked = false;

	world->profile.step = b2GetMilliseconds(&stepTimer);

	

	// Ensure stack is large enough
	b2GrowStack(world->stackAllocator);

	// Make sure all tasks that were started were also finished
	

	b2TracyCZoneEnd(world_step);
}

static void b2DrawShape(b2DebugDraw* draw, b2Shape* shape, Tran2 xf, Color color)
{
	switch (shape->type)
	{
		case b2_capsuleShape:
		{
			b2Capsule* capsule = &shape->capsule;
			Vec2 p1 = tran2_transform(xf, capsule->point1);
			Vec2 p2 = tran2_transform(xf, capsule->point2);
			draw->DrawSolidCapsule(p1, p2, capsule->radius, color, draw->context);
		}
		break;

		case b2_circleShape:
		{
			b2Circle* circle = &shape->circle;
			Vec2 center = tran2_transform(xf, circle->point);
			Vec2 axis = rot2_rotate(xf.rotation, vec2(1.0f, 0.0f));
			draw->DrawSolidCircle(center, circle->radius, axis, color, draw->context);
		}
		break;

		case b2_polygonShape:
		{
			Color fillColor = {0.5f * color.p0, 0.5f * color.p1, 0.5f * color.p2, 0.5f};

			b2Polygon* poly = &shape->polygon;
			int32_t count = poly->count;
			
			Vec2 vertices[b2_maxPolygonVertices];

			for (int32_t i = 0; i < count; ++i)
			{
				vertices[i] = tran2_transform(xf, poly->vertices[i]);
			}

			if (poly->radius > 0.0f)
			{
				draw->DrawRoundedPolygon(vertices, count, poly->radius, fillColor, color, draw->context);
			}
			else
			{
				draw->DrawSolidPolygon(vertices, count, color, draw->context);
			}
		}
		break;

		case b2_segmentShape:
		{
			b2Segment* segment = &shape->segment;
			Vec2 p1 = tran2_transform(xf, segment->point1);
			Vec2 p2 = tran2_transform(xf, segment->point2);
			draw->DrawSegment(p1, p2, color, draw->context);
		}
		break;

		case b2_smoothSegmentShape:
		{
			b2Segment* segment = &shape->smoothSegment.segment;
			Vec2 p1 = tran2_transform(xf, segment->point1);
			Vec2 p2 = tran2_transform(xf, segment->point2);
			draw->DrawSegment(p1, p2, color, draw->context);
			draw->DrawPoint(p2, 4.0f, color, draw->context);
			draw->DrawSegment(p1, vec2_lerp(p1, p2, 0.1f), b2MakeColor(b2_colorPaleGreen4, 1.0f), draw->context);
		}
		break;

		default:
			break;
	}
}

void b2World_Draw(b2WorldId worldId, b2DebugDraw* draw)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return;
	}

	if (draw->drawShapes)
	{
		int32_t count = world->bodyPool.capacity;
		for (int32_t i = 0; i < count; ++i)
		{
			b2Body* b = world->bodies + i;
			if (b->object.next != i)
			{
				continue;
			}

			bool isAwake = false;
			if (b->islandIndex != B2_NULL_INDEX)
			{
				isAwake = world->islands[b->islandIndex].awakeIndex != B2_NULL_INDEX;
			}

			Tran2 xf = b->transform;
			int32_t shapeIndex = b->shapeList;
			while (shapeIndex != B2_NULL_INDEX)
			{
				b2Shape* shape = world->shapes + shapeIndex;
				Color color;

				if (b->type == b2_dynamicBody && b->mass == 0.0f)
				{
					// Bad body
					color = b2MakeColor(b2_colorRed, 0.5f);
				}
				else if (b->isEnabled == false)
				{
					color = b2MakeColor(b2_colorSlateGray2, 0.5f);
				}
				else if (shape->isSensor)
				{
					color = b2MakeColor(b2_colorWheat, 1.0f);
				}
				else if (b->isSpeedCapped)
				{
					color = b2MakeColor(b2_colorYellow, 1.0f);
				}
				else if (b->isFast)
				{
					color = b2MakeColor(b2_colorSalmon, 1.0f);
				}
				else if (b->type == b2_staticBody)
				{
					color = b2MakeColor(b2_colorPaleGreen, 1.0f);
				}
				else if (b->type == b2_kinematicBody)
				{
					color = (Color){0.5f, 0.5f, 0.9f, 1.0f};
				}
				else if (isAwake)
				{
					color = b2MakeColor(b2_colorPink3, 1.0f);
				}
				else
				{
					color = b2MakeColor(b2_colorGray40, 1.0f);
				}

				b2DrawShape(draw, shape, xf, color);
				shapeIndex = shape->nextShapeIndex;
			}
		}
	}

	if (draw->drawJoints)
	{
		int32_t count = world->jointPool.capacity;
		for (int32_t i = 0; i < count; ++i)
		{
			b2Joint* joint = world->joints + i;
			if (joint->object.next != i)
			{
				continue;
			}

			b2DrawJoint(draw, world, joint);
		}
	}

	if (draw->drawAABBs)
	{
		Color color = {0.9f, 0.3f, 0.9f, 1.0f};

		int32_t count = world->bodyPool.capacity;
		for (int32_t i = 0; i < count; ++i)
		{
			b2Body* b = world->bodies + i;
			if (b->object.next != i)
			{
				continue;
			}

			char buffer[32];
			sprintf(buffer, "%d", b->object.index);
			draw->DrawString(b->position, buffer, draw->context);

			int32_t shapeIndex = b->shapeList;
			while (shapeIndex != B2_NULL_INDEX)
			{
				b2Shape* shape = world->shapes + shapeIndex;
				AABB aabb = shape->fatAABB;

				Vec2 vs[4] = {{aabb.min.x, aabb.min.y},
								{aabb.max.x, aabb.min.y},
								{aabb.max.x, aabb.max.y},
								{aabb.min.x, aabb.max.y}};

				draw->DrawPolygon(vs, 4, color, draw->context);

				shapeIndex = shape->nextShapeIndex;
			}
		}
	}

	if (draw->drawMass)
	{
		Vec2 offset = {0.1f, 0.1f};
		b2Body* bodies = world->bodies;
		int32_t bodyCapacity = world->bodyPool.capacity;
		for (int32_t i = 0; i < bodyCapacity; ++i)
		{
			b2Body* body = bodies + i;
			if (b2ObjectValid(&body->object) == false)
			{
				continue;
			}

			Tran2 transform = {body->position, body->transform.rotation};
			draw->DrawTransform(transform, draw->context);

			Vec2 p = tran2_transform(transform, offset);

			char buffer[32];
			sprintf(buffer, "%.2f", body->mass);
			draw->DrawString(p, buffer, draw->context);
		}
	}

	if (draw->drawContacts)
	{
		const float k_impulseScale = 1.0f;
		const float k_axisScale = 0.3f;
		Color speculativeColor = {0.3f, 0.3f, 0.3f, 1.0f};
		Color addColor = {0.3f, 0.95f, 0.3f, 1.0f};
		Color persistColor = {0.3f, 0.3f, 0.95f, 1.0f};
		Color normalColor = {0.9f, 0.9f, 0.9f, 1.0f};
		Color impulseColor = {0.9f, 0.9f, 0.3f, 1.0f};
		Color frictionColor = {0.9f, 0.9f, 0.3f, 1.0f};

		b2HexColor colors[b2_graphColorCount + 1] = {
			b2_colorRed,  b2_colorOrange,	 b2_colorYellow,	b2_colorGreen, b2_colorCyan, b2_colorBlue, b2_colorViolet,
			b2_colorPink, b2_colorChocolate, b2_colorGoldenrod, b2_colorCoral, b2_colorAqua, b2_colorBlack};

		int count = b2GetArrayCount(world->awakeContactArray);

		for (int32_t i = 0; i < count; ++i)
		{
			int index = world->awakeContactArray[i];
			if (index == B2_NULL_INDEX)
			{
				continue;
			}

			b2Contact* contact = world->contacts + index;
			int pointCount = contact->manifold.pointCount;
			int colorIndex = contact->colorIndex;
			Vec2 normal = contact->manifold.normal;
			char buffer[32];

			for (int j = 0; j < pointCount; ++j)
			{
				b2ManifoldPoint* point = contact->manifold.points + j;

				if (draw->drawGraphColors && 0 <= colorIndex && colorIndex <= b2_graphColorCount)
				{
					// graph color
					float pointSize = colorIndex == b2_graphColorCount ? 7.5f : 5.0f;
					draw->DrawPoint(point->point, pointSize, b2MakeColor(colors[colorIndex], 1.0f), draw->context);
					// g_draw.DrawString(point->position, "%d", point->color);
				}
				else if (point->separation > b2_linearSlop)
				{
					// Speculative
					draw->DrawPoint(point->point, 5.0f, speculativeColor, draw->context);
				}
				else if (point->persisted == false)
				{
					// Add
					draw->DrawPoint(point->point, 10.0f, addColor, draw->context);
				}
				else if (point->persisted == true)
				{
					// Persist
					draw->DrawPoint(point->point, 5.0f, persistColor, draw->context);
				}

				if (draw->drawContactNormals)
				{
					Vec2 p1 = point->point;
					Vec2 p2 = vec2_mul_add(p1, k_axisScale, normal);
					draw->DrawSegment(p1, p2, normalColor, draw->context);
				}
				else if (draw->drawContactImpulses)
				{
					Vec2 p1 = point->point;
					Vec2 p2 = vec2_mul_add(p1, k_impulseScale * point->normalImpulse, normal);
					draw->DrawSegment(p1, p2, impulseColor, draw->context);
					snprintf(buffer, B2_ARRAY_COUNT(buffer), "%.2f", point->normalImpulse);
					draw->DrawString(p1, buffer, draw->context);
				}

				if (draw->drawFrictionImpulses)
				{
					Vec2 tangent = vec2_perp_right(normal);
					Vec2 p1 = point->point;
					Vec2 p2 = vec2_mul_add(p1, k_impulseScale * point->tangentImpulse, tangent);
					draw->DrawSegment(p1, p2, frictionColor, draw->context);
					snprintf(buffer, B2_ARRAY_COUNT(buffer), "%.2f", point->normalImpulse);
					draw->DrawString(p1, buffer, draw->context);
				}
			}
		}
	}
}

b2SensorEvents b2World_GetSensorEvents(b2WorldId worldId)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return (b2SensorEvents){0};
	}

	int beginCount = b2Array(world->sensorBeginEventArray).count;
	int endCount = b2Array(world->sensorEndEventArray).count;

	b2SensorEvents events = {world->sensorBeginEventArray, world->sensorEndEventArray, beginCount, endCount};
	return events;
}

b2ContactEvents b2World_GetContactEvents(b2WorldId worldId)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return (b2ContactEvents){0};
	}

	int beginCount = b2Array(world->contactBeginArray).count;
	int endCount = b2Array(world->contactEndArray).count;

	b2ContactEvents events = {world->contactBeginArray, world->contactEndArray, beginCount, endCount};
	return events;
}

bool b2World_IsValid(b2WorldId id)
{
	if (id.index < 0 || b2_maxWorlds <= id.index)
	{
		return false;
	}

	b2World* world = b2_worlds + id.index;
	return id.revision == world->revision;
}

bool b2Body_IsValid(b2BodyId id)
{
	if (id.world < 0 || b2_maxWorlds <= id.world)
	{
		return false;
	}

	b2World* world = b2_worlds + id.world;

	if (id.index < 0 || world->bodyPool.capacity <= id.index)
	{
		return false;
	}

	b2Body* body = world->bodies + id.index;
	if (b2ObjectValid(&body->object) == false)
	{
		return false;
	}

	return id.revision == body->object.revision;
}

bool b2Shape_IsValid(b2ShapeId id)
{
	if (id.world < 0 || b2_maxWorlds <= id.world)
	{
		return false;
	}

	b2World* world = b2_worlds + id.world;

	if (id.index < 0 || world->shapePool.capacity <= id.index)
	{
		return false;
	}

	b2Shape* shape = world->shapes + id.index;
	if (b2ObjectValid(&shape->object) == false)
	{
		return false;
	}

	return id.revision == shape->object.revision;
}

bool b2Chain_IsValid(b2ChainId id)
{
	if (id.world < 0 || b2_maxWorlds <= id.world)
	{
		return false;
	}

	b2World* world = b2_worlds + id.world;

	if (id.index < 0 || world->chainPool.capacity <= id.index)
	{
		return false;
	}

	b2ChainShape* chain = world->chains + id.index;
	if (b2ObjectValid(&chain->object) == false)
	{
		return false;
	}

	return id.revision == chain->object.revision;
}

bool b2Joint_IsValid(b2JointId id)
{
	if (id.world < 0 || b2_maxWorlds <= id.world)
	{
		return false;
	}

	b2World* world = b2_worlds + id.world;

	if (id.index < 0 || world->jointPool.capacity <= id.index)
	{
		return false;
	}

	b2Joint* joint = world->joints + id.index;
	if (b2ObjectValid(&joint->object) == false)
	{
		return false;
	}

	return id.revision == joint->object.revision;
}

void b2World_EnableSleeping(b2WorldId worldId, bool flag)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return;
	}

	if (flag == world->enableSleep)
	{
		return;
	}

	world->enableSleep = flag;
	if (flag == false)
	{
		int32_t count = world->islandPool.capacity;
		for (int32_t i = 0; i < count; ++i)
		{
			b2Island* island = world->islands + i;
			if (island->object.next != i)
			{
				continue;
			}

			b2WakeIsland(island);
		}
	}
}

void b2World_EnableWarmStarting(b2WorldId worldId, bool flag)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return;
	}

	world->enableWarmStarting = flag;
}

void b2World_EnableContinuous(b2WorldId worldId, bool flag)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return;
	}

	world->enableContinuous = flag;
}

void b2World_SetRestitutionThreshold(b2WorldId worldId, float value)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return;
	}

	world->restitutionThreshold = clampf(value, 0.0f, FLT_MAX);
}

void b2World_SetContactTuning(b2WorldId worldId, float hertz, float dampingRatio, float pushOut)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return;
	}

	world->contactHertz = clampf(hertz, 0.0f, FLT_MAX);
	world->contactDampingRatio = clampf(dampingRatio, 0.0f, FLT_MAX);
	world->contactPushoutVelocity = clampf(pushOut, 0.0f, FLT_MAX);
}

b2Profile b2World_GetProfile(b2WorldId worldId)
{
	b2World* world = b2GetWorldFromId(worldId);
	return world->profile;
}

b2Counters b2World_GetCounters(b2WorldId worldId)
{
	b2World* world = b2GetWorldFromId(worldId);
	b2Counters s = {0};
	s.islandCount = world->islandPool.count;
	s.bodyCount = world->bodyPool.count;
	s.contactCount = world->contactPool.count;
	s.jointCount = world->jointPool.count;
	s.pairCount = world->broadPhase.pairSet.count;

	b2DynamicTree* tree = world->broadPhase.trees + b2_dynamicBody;
	s.proxyCount = tree->nodeCount;
	s.treeHeight = b2DynamicTree_GetHeight(tree);
	s.stackCapacity = b2GetStackCapacity(world->stackAllocator);
	s.stackUsed = b2GetMaxStackAllocation(world->stackAllocator);
	s.taskCount = world->taskCount;
	for (int32_t i = 0; i <= b2_graphColorCount; ++i)
	{
		s.colorCounts[i] = world->graph.occupancy[i];
	}
	return s;
}

typedef struct WorldQueryContext
{
	b2World* world;
	b2QueryResultFcn* fcn;
	b2QueryFilter filter;
	void* userContext;
} WorldQueryContext;

static bool TreeQueryCallback(int32_t proxyId, int32_t shapeIndex, void* context)
{
	B2_MAYBE_UNUSED(proxyId);

	WorldQueryContext* worldContext = context;
	b2World* world = worldContext->world;

	

	b2Shape* shape = world->shapes + shapeIndex;
	b2Filter shapeFilter = shape->filter;
	b2QueryFilter queryFilter = worldContext->filter;

	if ((shapeFilter.categoryBits & queryFilter.maskBits) == 0 || (shapeFilter.maskBits & queryFilter.categoryBits) == 0)
	{
		return true;
	}

	

	b2ShapeId shapeId = {shape->object.index, world->index, shape->object.revision};
	bool result = worldContext->fcn(shapeId, worldContext->userContext);
	return result;
}

void b2World_QueryAABB(b2WorldId worldId, b2QueryResultFcn* fcn, AABB aabb, b2QueryFilter filter, void* context)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return;
	}

	WorldQueryContext worldContext = {world, fcn, filter, context};

	for (int32_t i = 0; i < b2_bodyTypeCount; ++i)
	{
		b2DynamicTree_Query(world->broadPhase.trees + i, aabb, TreeQueryCallback, &worldContext);
	}
}

typedef struct WorldOverlapContext
{
	b2World* world;
	b2QueryResultFcn* fcn;
	b2QueryFilter filter;
	b2DistanceProxy proxy;
	Tran2 transform;
	void* userContext;
} WorldOverlapContext;

static bool TreeOverlapCallback(int32_t proxyId, int32_t shapeIndex, void* context)
{
	B2_MAYBE_UNUSED(proxyId);

	WorldOverlapContext* worldContext = context;
	b2World* world = worldContext->world;

	

	b2Shape* shape = world->shapes + shapeIndex;
	b2Filter shapeFilter = shape->filter;
	b2QueryFilter queryFilter = worldContext->filter;

	if ((shapeFilter.categoryBits & queryFilter.maskBits) == 0 || (shapeFilter.maskBits & queryFilter.categoryBits) == 0)
	{
		return true;
	}

	

	b2DistanceInput input;
	input.proxyA = worldContext->proxy;
	input.proxyB = b2MakeShapeDistanceProxy(shape);
	input.transformA = worldContext->transform;
	input.transformB = world->bodies[shape->bodyIndex].transform;
	input.useRadii = true;

	b2DistanceCache cache = {0};
	b2DistanceOutput output = b2ShapeDistance(&cache, &input);

	if (output.distance > 0.0f)
	{
		return true;
	}

	b2ShapeId shapeId = {shape->object.index, world->index, shape->object.revision};
	bool result = worldContext->fcn(shapeId, worldContext->userContext);
	return result;
}

void b2World_OverlapCircle(b2WorldId worldId, b2QueryResultFcn* fcn, const b2Circle* circle, Tran2 transform,
						   b2QueryFilter filter, void* context)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return;
	}

	AABB aabb = b2ComputeCircleAABB(circle, transform);
	WorldOverlapContext worldContext = {
		world, fcn, filter, b2MakeProxy(&circle->point, 1, circle->radius), transform, context,
	};

	for (int32_t i = 0; i < b2_bodyTypeCount; ++i)
	{
		b2DynamicTree_Query(world->broadPhase.trees + i, aabb, TreeOverlapCallback, &worldContext);
	}
}

void b2World_OverlapCapsule(b2WorldId worldId, b2QueryResultFcn* fcn, const b2Capsule* capsule, Tran2 transform,
							b2QueryFilter filter, void* context)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return;
	}

	AABB aabb = b2ComputeCapsuleAABB(capsule, transform);
	WorldOverlapContext worldContext = {
		world, fcn, filter, b2MakeProxy(&capsule->point1, 2, capsule->radius), transform, context,
	};

	for (int32_t i = 0; i < b2_bodyTypeCount; ++i)
	{
		b2DynamicTree_Query(world->broadPhase.trees + i, aabb, TreeOverlapCallback, &worldContext);
	}
}

void b2World_OverlapPolygon(b2WorldId worldId, b2QueryResultFcn* fcn, const b2Polygon* polygon, Tran2 transform,
							b2QueryFilter filter, void* context)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return;
	}

	AABB aabb = b2ComputePolygonAABB(polygon, transform);
	WorldOverlapContext worldContext = {
		world, fcn, filter, b2MakeProxy(polygon->vertices, polygon->count, polygon->radius), transform, context,
	};

	for (int32_t i = 0; i < b2_bodyTypeCount; ++i)
	{
		b2DynamicTree_Query(world->broadPhase.trees + i, aabb, TreeOverlapCallback, &worldContext);
	}
}

typedef struct WorldRayCastContext
{
	b2World* world;
	b2RayResultFcn* fcn;
	b2QueryFilter filter;
	float fraction;
	void* userContext;
} WorldRayCastContext;

static float RayCastCallback(const b2RayCastInput* input, int32_t proxyId, int32_t shapeIndex, void* context)
{
	B2_MAYBE_UNUSED(proxyId);

	WorldRayCastContext* worldContext = context;
	b2World* world = worldContext->world;

	

	b2Shape* shape = world->shapes + shapeIndex;
	b2Filter shapeFilter = shape->filter;
	b2QueryFilter queryFilter = worldContext->filter;

	if ((shapeFilter.categoryBits & queryFilter.maskBits) == 0 || (shapeFilter.maskBits & queryFilter.categoryBits) == 0)
	{
		return input->maxFraction;
	}

	int32_t bodyIndex = shape->bodyIndex;
	

	b2Body* body = world->bodies + bodyIndex;
	

	b2RayCastOutput output = b2RayCastShape(input, shape, body->transform);

	if (output.hit)
	{
		b2ShapeId shapeId = {shapeIndex, world->index, shape->object.revision};
		float fraction = worldContext->fcn(shapeId, output.point, output.normal, output.fraction, worldContext->userContext);
		worldContext->fraction = fraction;
		return fraction;
	}

	return input->maxFraction;
}

void b2World_RayCast(b2WorldId worldId, Vec2 origin, Vec2 translation, b2QueryFilter filter, b2RayResultFcn* fcn,
					 void* context)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return;
	}

	b2RayCastInput input = {origin, translation, 1.0f};

	// todo validate input

	WorldRayCastContext worldContext = {world, fcn, filter, 1.0f, context};

	for (int32_t i = 0; i < b2_bodyTypeCount; ++i)
	{
		b2DynamicTree_RayCast(world->broadPhase.trees + i, &input, filter.maskBits, RayCastCallback, &worldContext);

		if (worldContext.fraction == 0.0f)
		{
			return;
		}

		input.maxFraction = worldContext.fraction;
	}
}

// This callback finds the closest hit. This is the most common callback used in games.
static float b2RayCastClosestFcn(b2ShapeId shapeId, Vec2 point, Vec2 normal, float fraction, void* context)
{
	b2RayResult* rayResult = (b2RayResult*)context;
	rayResult->shapeId = shapeId;
	rayResult->point = point;
	rayResult->normal = normal;
	rayResult->fraction = fraction;
	rayResult->hit = true;
	return fraction;
}

b2RayResult b2World_RayCastClosest(b2WorldId worldId, Vec2 origin, Vec2 translation, b2QueryFilter filter)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return b2_emptyRayResult;
	}

	b2RayCastInput input = {origin, translation, 1.0f};
	b2RayResult result = b2_emptyRayResult;
	WorldRayCastContext worldContext = {world, b2RayCastClosestFcn, filter, 1.0f, &result};

	for (int32_t i = 0; i < b2_bodyTypeCount; ++i)
	{
		b2DynamicTree_RayCast(world->broadPhase.trees + i, &input, filter.maskBits, RayCastCallback, &worldContext);

		if (worldContext.fraction == 0.0f)
		{
			return result;
		}

		input.maxFraction = worldContext.fraction;
	}

	return result;
}

static float ShapeCastCallback(const b2ShapeCastInput* input, int32_t proxyId, int32_t shapeIndex, void* context)
{
	B2_MAYBE_UNUSED(proxyId);

	WorldRayCastContext* worldContext = context;
	b2World* world = worldContext->world;

	

	b2Shape* shape = world->shapes + shapeIndex;
	b2Filter shapeFilter = shape->filter;
	b2QueryFilter queryFilter = worldContext->filter;

	if ((shapeFilter.categoryBits & queryFilter.maskBits) == 0 || (shapeFilter.maskBits & queryFilter.categoryBits) == 0)
	{
		return input->maxFraction;
	}

	int32_t bodyIndex = shape->bodyIndex;
	

	b2Body* body = world->bodies + bodyIndex;
	

	b2RayCastOutput output = b2ShapeCastShape(input, shape, body->transform);

	if (output.hit)
	{
		b2ShapeId shapeId = {shapeIndex, world->index, shape->object.revision};
		float fraction = worldContext->fcn(shapeId, output.point, output.normal, output.fraction, worldContext->userContext);
		worldContext->fraction = fraction;
		return fraction;
	}

	return input->maxFraction;
}

void b2World_CircleCast(b2WorldId worldId, const b2Circle* circle, Tran2 originTransform, Vec2 translation,
						b2QueryFilter filter, b2RayResultFcn* fcn, void* context)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return;
	}

	b2ShapeCastInput input;
	input.points[0] = tran2_transform(originTransform, circle->point);
	input.count = 1;
	input.radius = circle->radius;
	input.translation = translation;
	input.maxFraction = 1.0f;

	WorldRayCastContext worldContext = {world, fcn, filter, 1.0f, context};

	for (int32_t i = 0; i < b2_bodyTypeCount; ++i)
	{
		b2DynamicTree_ShapeCast(world->broadPhase.trees + i, &input, filter.maskBits, ShapeCastCallback, &worldContext);

		if (worldContext.fraction == 0.0f)
		{
			return;
		}

		input.maxFraction = worldContext.fraction;
	}
}

void b2World_CapsuleCast(b2WorldId worldId, const b2Capsule* capsule, Tran2 originTransform, Vec2 translation,
						 b2QueryFilter filter, b2RayResultFcn* fcn, void* context)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return;
	}

	b2ShapeCastInput input;
	input.points[0] = tran2_transform(originTransform, capsule->point1);
	input.points[1] = tran2_transform(originTransform, capsule->point2);
	input.count = 2;
	input.radius = capsule->radius;
	input.translation = translation;
	input.maxFraction = 1.0f;

	WorldRayCastContext worldContext = {world, fcn, filter, 1.0f, context};

	for (int32_t i = 0; i < b2_bodyTypeCount; ++i)
	{
		b2DynamicTree_ShapeCast(world->broadPhase.trees + i, &input, filter.maskBits, ShapeCastCallback, &worldContext);

		if (worldContext.fraction == 0.0f)
		{
			return;
		}

		input.maxFraction = worldContext.fraction;
	}
}

void b2World_PolygonCast(b2WorldId worldId, const b2Polygon* polygon, Tran2 originTransform, Vec2 translation,
						 b2QueryFilter filter, b2RayResultFcn* fcn, void* context)
{
	b2World* world = b2GetWorldFromId(worldId);
	
	if (world->locked)
	{
		return;
	}

	b2ShapeCastInput input;
	for (int i = 0; i < polygon->count; ++i)
	{
		input.points[i] = tran2_transform(originTransform, polygon->vertices[i]);
	}
	input.count = polygon->count;
	input.radius = polygon->radius;
	input.translation = translation;
	input.maxFraction = 1.0f;

	WorldRayCastContext worldContext = {world, fcn, filter, 1.0f, context};

	for (int32_t i = 0; i < b2_bodyTypeCount; ++i)
	{
		b2DynamicTree_ShapeCast(world->broadPhase.trees + i, &input, filter.maskBits, ShapeCastCallback, &worldContext);

		if (worldContext.fraction == 0.0f)
		{
			return;
		}

		input.maxFraction = worldContext.fraction;
	}
}

#if 0

void b2World_ShiftOrigin(b2WorldId worldId, Vec2 newOrigin)
{
	
	if (m_locked)
	{
		return;
	}

	for (b2Body* b = m_bodyList; b; b = b->m_next)
	{
		b->m_xf.position -= newOrigin;
		b->m_sweep.c0 -= newOrigin;
		b->m_sweep.c -= newOrigin;
	}

	for (b2Joint* j = m_jointList; j; j = j->m_next)
	{
		j->ShiftOrigin(newOrigin);
	}

	m_contactManager.m_broadPhase.ShiftOrigin(newOrigin);
}

void b2World_Dump()
{
	if (m_locked)
	{
		return;
	}

	b2OpenDump("box2d_dump.inl");

	b2Dump("Vec2 g(%.9g, %.9g);\n", m_gravity.x, m_gravity.y);
	b2Dump("m_world->SetGravity(g);\n");

	b2Dump("b2Body** bodies = (b2Body**)xxmalloc(%d * sizeof(b2Body*));\n", m_bodyCount);
	b2Dump("b2Joint** joints = (b2Joint**)xxmalloc(%d * sizeof(b2Joint*));\n", m_jointCount);

	int32 i = 0;
	for (b2Body* b = m_bodyList; b; b = b->m_next)
	{
		b->m_islandIndex = i;
		b->Dump();
		++i;
	}

	i = 0;
	for (b2Joint* j = m_jointList; j; j = j->m_next)
	{
		j->m_index = i;
		++i;
	}

	// First pass on joints, skip gear joints.
	for (b2Joint* j = m_jointList; j; j = j->m_next)
	{
		if (j->m_type == e_gearJoint)
		{
			continue;
		}

		b2Dump("{\n");
		j->Dump();
		b2Dump("}\n");
	}

	// Second pass on joints, only gear joints.
	for (b2Joint* j = m_jointList; j; j = j->m_next)
	{
		if (j->m_type != e_gearJoint)
		{
			continue;
		}

		b2Dump("{\n");
		j->Dump();
		b2Dump("}\n");
	}

	b2Dump("xxfree(joints);\n");
	b2Dump("xxfree(bodies);\n");
	b2Dump("joints = nullptr;\n");
	b2Dump("bodies = nullptr;\n");

	b2CloseDump();
}
#endif

bool b2IsBodyIdValid(b2World* world, b2BodyId id)
{
	if (id.world != world->index)
	{
		return false;
	}

	if (id.index >= world->bodyPool.capacity)
	{
		return false;
	}

	b2Body* body = world->bodies + id.index;
	if (body->object.index != body->object.next)
	{
		return false;
	}

	if (body->object.revision != id.revision)
	{
		return false;
	}

	return true;
}

void b2World_SetPreSolveCallback(b2WorldId worldId, b2PreSolveFcn* fcn, void* context)
{
	b2World* world = b2GetWorldFromId(worldId);
	world->preSolveFcn = fcn;
	world->preSolveContext = context;
}
