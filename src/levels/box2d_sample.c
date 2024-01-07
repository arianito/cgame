#include "graph1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/camera.h"
#include "engine/draw.h"
#include "engine/input.h"
#include "engine/debug.h"
#include "engine/sprite.h"
#include "engine/atlas.h"
//
#include "mem/alloc.h"
//
#include "math/rot2.h"
//
#include "box2d/id.h"
#include "box2d/box2d.h"
#include "box2d/world.h"
#include "box2d/joint_util.h"
#include "box2d/debug_draw.h"
#include "box2d_debug_draw.h"

typedef struct
{
    b2DebugDraw debug;
    b2WorldId worldId;
    b2JointId mouseJoint;
    b2BodyId groundId;
} Sample2dContext;

static void create(Sample2dContext *self)
{
    b2WorldDef worldDef = b2_defaultWorldDef;
    worldDef.bodyCapacity = 2;
    worldDef.contactCapacity = 2;
    worldDef.arenaAllocatorCapacity = 0;

    self->mouseJoint = b2_nullJointId;
    self->groundId = b2_nullBodyId;

    self->worldId = b2CreateWorld(&worldDef);
    b2World *world = b2GetWorldFromId(self->worldId);

    b2DebugDraw *draw = &self->debug;
    draw->drawShapes = true;
    draw->drawJoints = true;

    draw->DrawPolygon = &dbgDrawPolygon;
    draw->DrawSolidPolygon = &dbgDrawSolidPolygon;
    draw->DrawRoundedPolygon = &dbgDrawRoundedPolygon;
    draw->DrawCircle = &dbgDrawCircle;
    draw->DrawSolidCircle = &dbgDrawSolidCircle;
    draw->DrawCapsule = &dbgDrawCapsule;
    draw->DrawSolidCapsule = &dbgDrawSolidCapsule;
    draw->DrawSegment = &dbgDrawSegment;
    draw->DrawTransform = &dbgDrawTransform;
    draw->DrawPoint = &dbgDrawPoint;
    draw->DrawString = &dbgDrawString;

    {
        b2BodyDef bodyDef = b2_defaultBodyDef;
        b2BodyId groundId = b2CreateBody(self->worldId, &bodyDef);
        b2Segment segment = {{-60.0, 0.0}, {60.0, 0.0}};
        b2ShapeDef shapeDef = b2_defaultShapeDef;
        b2CreateSegmentShape(groundId, &shapeDef, &segment);
    }
    {
        b2BodyDef bodyDef = b2_defaultBodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = vec2(30.0, 10.0);
        b2BodyId attachmentId = b2CreateBody(self->worldId, &bodyDef);
        b2Polygon box = b2MakeBox(3.0, 10.0);
        b2ShapeDef shapeDef = b2_defaultShapeDef;
        shapeDef.density = 10.0;
        b2CreatePolygonShape(attachmentId, &shapeDef, &box);
    }
    {
        b2BodyDef bodyDef = b2_defaultBodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = vec2(-30.0, 10.0);
        b2BodyId attachmentId = b2CreateBody(self->worldId, &bodyDef);
        b2Polygon box = b2MakeBox(3.0, 10.0);
        b2ShapeDef shapeDef = b2_defaultShapeDef;
        shapeDef.density = 10.0;
        b2CreatePolygonShape(attachmentId, &shapeDef, &box);
    }
    for (int i = 0; i < 10; i++)
    {
        for (int j = -5; j <= 5; j++)
        {

            b2BodyDef bodyDef = b2_defaultBodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.position = vec2(j * 4, 3 + i * 3);
            b2BodyId attachmentId = b2CreateBody(self->worldId, &bodyDef);
            {

                b2Circle circle = {{1, 0}, 1.0};
                b2ShapeDef shapeDef = b2_defaultShapeDef;
                shapeDef.density = 1.0;
                b2CreateCircleShape(attachmentId, &shapeDef, &circle);
            }
            {

                b2Circle circle = {{-1, 0}, 1.0};
                b2ShapeDef shapeDef = b2_defaultShapeDef;
                shapeDef.density = 1.0;
                b2CreateCircleShape(attachmentId, &shapeDef, &circle);
            }
        }
    }
}

typedef struct
{
    Vec2 point;
    b2BodyId bodyId;
} QueryContext;

bool QueryCallback(b2ShapeId shapeId, void *context)
{
    QueryContext *queryContext = (QueryContext *)(context);
    b2BodyId bodyId = b2Shape_GetBody(shapeId);
    b2BodyType bodyType = b2Body_GetType(bodyId);
    if (bodyType != b2_dynamicBody)
    {
        return true;
    }
    bool overlap = b2Shape_TestPoint(shapeId, queryContext->point);
    if (overlap)
    {
        queryContext->bodyId = bodyId;
        return false;
    }
    return true;
}
static void render(Sample2dContext *self)
{
    Ray r = camera_screenToWorld(input->position);
    Vec3 wp = vec3_intersect_plane(r.origin, vec3_mulf(r.direction, 1000), vec3_zero, vec3_forward);
    Vec2 p = vec2yz(wp);

    if (B2_IS_NULL(self->mouseJoint) && input_mousedown(MOUSE_LEFT))
    {

        QueryContext queryContext = {p, b2_nullBodyId};
        Vec2 d = vec2(0.001f, 0.001f);
        AABB box = aabb(vec2_sub(p, d), vec2_add(p, d));
        b2World_QueryAABB(self->worldId, QueryCallback, box, b2_defaultQueryFilter, &queryContext);

        if (B2_NON_NULL(queryContext.bodyId))
        {
            float frequencyHz = 4.0f;
            float dampingRatio = 0.6f;
            float mass = b2Body_GetMass(queryContext.bodyId);

            self->groundId = b2CreateBody(self->worldId, &b2_defaultBodyDef);

            b2MouseJointDef jd = b2_defaultMouseJointDef;
            jd.bodyIdA = self->groundId;
            jd.bodyIdB = queryContext.bodyId;
            jd.target = p;
            jd.maxForce = 1000.0f * mass;
            b2LinearStiffness(&jd.stiffness, &jd.damping, frequencyHz, dampingRatio, self->groundId, queryContext.bodyId);

            self->mouseJoint = b2CreateMouseJoint(self->worldId, &jd);

            b2Body_Wake(queryContext.bodyId);
        }
    }
    if (B2_NON_NULL(self->mouseJoint))
    {
        b2MouseJoint_SetTarget(self->mouseJoint, p);
        b2BodyId bodyIdB = b2Joint_GetBodyB(self->mouseJoint);
        b2Body_Wake(bodyIdB);

        if (input_mouseup(MOUSE_LEFT))
        {
            b2DestroyJoint(self->mouseJoint);
            self->mouseJoint = b2_nullJointId;

            b2DestroyBody(self->groundId);
            self->groundId = b2_nullBodyId;
        }
    }

    b2World_Step(self->worldId, gtime->delta * 2, 8, 3);
    b2World_Draw(self->worldId, &self->debug);
}

static void destroy(Sample2dContext *self)
{
    sprite_clear();
    atlas_clear();
    b2DestroyWorld(self->worldId);
}

Level make_box2dsample()
{
    return (Level){
        context : arena_alloc(alloc->global, sizeof(Sample2dContext)),
        create : &create,
        render : &render,
        destroy : &destroy,
    };
}