// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#include "body.h"
#include "core.h"
#include "joint.h"
#include "solver_data.h"
#include "world.h"

// needed for dll export
#include "box2d/box2d.h"

// p = attached point, m = mouse point
// C = p - m
// Cdot = v
//      = v + cross(w, r)
// J = [I r_skew]
// Identity used:
// w k % (rx i + ry mouse) = w * (-ry i + rx mouse)

void b2MouseJoint_SetTarget(b2JointId jointId, Vec2 target)
{
	b2World* world = b2GetWorldFromIndex(jointId.world);
	
	if (world->locked)
	{
		return;
	}

	

	b2Joint* base = world->joints + jointId.index;
	
	
	
	base->mouseJoint.targetA = target;
}

void b2PrepareMouseJoint(b2Joint* base, b2StepContext* context)
{
	

	int32_t indexB = base->edges[1].bodyIndex;
	

	b2Body* bodyB = context->bodies + indexB;
	

	b2MouseJoint* joint = &base->mouseJoint;
	joint->indexB = context->bodyToSolverMap[indexB];
	joint->localCenterB = bodyB->localCenter;

	Vec2 cB = bodyB->position;
	Rot2 qB = bodyB->transform.rotation;
	float mB = bodyB->invMass;
	float iB = bodyB->invI;

	float d = joint->damping;
	float k = joint->stiffness;

	// TODO_ERIN convert to bias/mass/impulse scales
	// magic formulas
	// gamma has units of inverse mass.
	// beta has units of inverse time.
	float h = context->dt;
	joint->gamma = h * (d + h * k);
	if (joint->gamma != 0.0f)
	{
		joint->gamma = 1.0f / joint->gamma;
	}
	joint->beta = h * k * joint->gamma;

	// Compute the effective mass matrix.
	joint->rB = rot2_rotate(qB, vec2_sub(base->localAnchorB, joint->localCenterB));

	// K    = [(1/m1 + 1/m2) * eye(2) - skew(r1) * invI1 * skew(r1) - skew(r2) * invI2 * skew(r2)]
	//      = [1/m1+1/m2     0    ] + invI1 * [r1.y*r1.y -r1.x*r1.y] + invI2 * [r1.y*r1.y -r1.x*r1.y]
	//        [    0     1/m1+1/m2]           [-r1.x*r1.y r1.x*r1.x]           [-r1.x*r1.y r1.x*r1.x]
	Mat2 K;
	K.cx.x = mB + iB * joint->rB.y * joint->rB.y + joint->gamma;
	K.cx.y = -iB * joint->rB.x * joint->rB.y;
	K.cy.x = K.cx.y;
	K.cy.y = mB + iB * joint->rB.x * joint->rB.x + joint->gamma;

	joint->mass = mat2_inv(K);

	joint->C = vec2_add(cB, vec2_sub(joint->rB, joint->targetA));
	joint->C = vec2_mulfv(joint->beta, joint->C);

	if (context->enableWarmStarting == false)
	{
		joint->impulse = vec2_zero;
	}
}

void b2WarmStartMouseJoint(b2Joint* base, b2StepContext* context)
{
	

	b2MouseJoint* joint = &base->mouseJoint;

	b2SolverBody* bodyB = context->solverBodies + joint->indexB;
	Vec2 vB = bodyB->linearVelocity;
	float wB = bodyB->angularVelocity;

	float mB = bodyB->invMass;
	float iB = bodyB->invI;

	// TODO_ERIN damp angular velocity?
	// wB *= 1.0f / (1.0f + 0.02f * context->dt);

	joint->impulse = vec2_mulfv(context->dtRatio, joint->impulse);
	vB = vec2_mul_add(vB, mB, joint->impulse);
	wB += iB * vec2_cross(joint->rB, joint->impulse);

	bodyB->linearVelocity = vB;
	bodyB->angularVelocity = wB;
}

void b2SolveMouseJoint(b2Joint* base, b2StepContext* context)
{
	b2MouseJoint* joint = &base->mouseJoint;
	b2SolverBody* bodyB = context->solverBodies + joint->indexB;

	Vec2 vB = bodyB->linearVelocity;
	float wB = bodyB->angularVelocity;

	// dv = v + cross(w, r)
	Vec2 dv = vec2_add(vB, vec2_crossfv(wB, joint->rB));
	Vec2 Cdot = vec2_add(dv, vec2_mul_add(joint->C, joint->gamma, joint->impulse));
	Vec2 impulse = vec2_neg(mat2_mulv(joint->mass, Cdot));

	Vec2 oldImpulse = joint->impulse;
	joint->impulse = vec2_add(joint->impulse, impulse);
	float maxImpulse = context->dt * joint->maxForce;
	if (vec2_sqr_length(joint->impulse) > maxImpulse * maxImpulse)
	{
		joint->impulse = vec2_mulfv(maxImpulse / vec2_length(joint->impulse), joint->impulse);
	}
	impulse = vec2_sub(joint->impulse, oldImpulse);

	vB = vec2_mul_add(vB, bodyB->invMass, impulse);
	wB += bodyB->invI * vec2_cross(joint->rB, impulse);

	bodyB->linearVelocity = vB;
	bodyB->angularVelocity = wB;
}
