// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#define _CRT_SECURE_NO_WARNINGS

#include "body.h"
#include "core.h"
#include "joint.h"
#include "solver_data.h"
#include "world.h"

// needed for dll export
#include "box2d/box2d.h"
#include "box2d/debug_draw.h"

#include "math/vec2.h"
#include "math/tran2.h"
#include "math/rot2.h"

#include <stdio.h>

// 1-D constrained system
// m (v2 - v1) = lambda
// v2 + (beta/h) * x1 + gamma * lambda = 0, gamma has units of inverse mass.
// x2 = x1 + h * v2

// 1-D mass-damper-spring system
// m (v2 - v1) + h * d * v2 + h * k *

// C = norm(p2 - p1) - L
// u = (p2 - p1) / norm(p2 - p1)
// Cdot = dot(u, v2 + cross(w2, r2) - v1 - cross(w1, r1))
// J = [-u -cross(r1, u) u cross(r2, u)]
// K = J * invM * JT
//   = invMass1 + invI1 * cross(r1, u)^2 + invMass2 + invI2 * cross(r2, u)^2

void b2PrepareDistanceJoint(b2Joint* base, b2StepContext* context)
{
	

	int32_t indexA = base->edges[0].bodyIndex;
	int32_t indexB = base->edges[1].bodyIndex;
	b2Body* bodyA = context->bodies + indexA;
	b2Body* bodyB = context->bodies + indexB;

	
	

	b2DistanceJoint* joint = &base->distanceJoint;

	joint->indexA = context->bodyToSolverMap[indexA];
	joint->indexB = context->bodyToSolverMap[indexB];

	float mA = bodyA->invMass;
	float iA = bodyA->invI;
	float mB = bodyB->invMass;
	float iB = bodyB->invI;

	// Compute the effective masses.
	joint->rA = rot2_rotate(bodyA->transform.rotation, vec2_sub(base->localAnchorA, bodyA->localCenter));
	joint->rB = rot2_rotate(bodyB->transform.rotation, vec2_sub(base->localAnchorB, bodyB->localCenter));
	joint->separation = vec2_add(vec2_sub(joint->rB, joint->rA), vec2_sub(bodyB->position, bodyA->position));

	Vec2 rA = joint->rA;
	Vec2 rB = joint->rB;

	Vec2 axis = vec2_norm(joint->separation);

	float crA = vec2_cross(rA, axis);
	float crB = vec2_cross(rB, axis);
	float k = mA + mB + iA * crA * crA + iB * crB * crB;
	joint->axialMass = k > 0.0f ? 1.0f / k : 0.0f;

	float dt = context->dt;

	// Spring parameters
	if (joint->hertz > 0.0f)
	{
		float omega = 2.0f * b2_pi * joint->hertz;
		float a1 = 2.0f * joint->dampingRatio + dt * omega;
		float a2 = dt * omega * a1;
		float a3 = 1.0f / (1.0f + a2);
		joint->springBiasCoefficient = omega / a1;
		joint->springImpulseCoefficient = a3;
		joint->springMassCoefficient = a2 * a3;
	}
	else
	{
		joint->springBiasCoefficient = 0.0f;
		joint->springImpulseCoefficient = 0.0f;
		joint->springMassCoefficient = 0.0f;
	}

	// Limit parameters
	{
		// as rigid as possible: hertz = 1/4 * substep Hz
		float hertz = 0.25f * context->velocityIterations * context->inv_dt;
		float zeta = 1.0f;

		float omega = 2.0f * b2_pi * hertz;
		float a1 = 2.0f * zeta + dt * omega;
		float a2 = dt * omega * a1;
		float a3 = 1.0f / (1.0f + a2);
		joint->limitBiasCoefficient = omega / a1;
		joint->limitImpulseCoefficient = a3;
		joint->limitMassCoefficient = a2 * a3;
	}

	if (context->enableWarmStarting)
	{
		float dtRatio = context->dtRatio;

		// Soft step works best when bilateral constraints have no warm starting.
		joint->impulse = 0.0f;
		joint->lowerImpulse *= dtRatio;
		joint->upperImpulse *= dtRatio;
	}
	else
	{
		joint->impulse = 0.0f;
		joint->lowerImpulse = 0.0f;
		joint->upperImpulse = 0.0f;
	}
}

void b2WarmStartDistanceJoint(b2Joint* base, b2StepContext* context)
{
	

	b2DistanceJoint* joint = &base->distanceJoint;

	// This is a dummy body to represent a static body since static bodies don't have a solver body.
	b2SolverBody dummyBody = {0};

	// Note: must warm start solver bodies
	b2SolverBody* bodyA = joint->indexA == B2_NULL_INDEX ? &dummyBody : context->solverBodies + joint->indexA;
	float mA = bodyA->invMass;
	float iA = bodyA->invI;

	b2SolverBody* bodyB = joint->indexB == B2_NULL_INDEX ? &dummyBody : context->solverBodies + joint->indexB;
	float mB = bodyB->invMass;
	float iB = bodyB->invI;

	Vec2 rA = joint->rA;
	Vec2 rB = joint->rB;

	Vec2 axis = vec2_norm(joint->separation);

	float axialImpulse = joint->impulse + joint->lowerImpulse - joint->upperImpulse;
	Vec2 P = vec2_mulfv(axialImpulse, axis);

	bodyA->linearVelocity = vec2_mul_sub(bodyA->linearVelocity, mA, P);
	bodyA->angularVelocity -= iA * vec2_cross(rA, P);
	bodyB->linearVelocity = vec2_mul_add(bodyB->linearVelocity, mB, P);
	bodyB->angularVelocity += iB * vec2_cross(rB, P);
}

void b2SolveDistanceJoint(b2Joint* base, b2StepContext* context, bool useBias)
{
	

	b2DistanceJoint* joint = &base->distanceJoint;

	// This is a dummy body to represent a static body since static bodies don't have a solver body.
	b2SolverBody dummyBody = {0};

	b2SolverBody* bodyA = joint->indexA == B2_NULL_INDEX ? &dummyBody : context->solverBodies + joint->indexA;
	Vec2 vA = bodyA->linearVelocity;
	float wA = bodyA->angularVelocity;
	float mA = bodyA->invMass;
	float iA = bodyA->invI;

	b2SolverBody* bodyB = joint->indexB == B2_NULL_INDEX ? &dummyBody : context->solverBodies + joint->indexB;
	Vec2 vB = bodyB->linearVelocity;
	float wB = bodyB->angularVelocity;
	float mB = bodyB->invMass;
	float iB = bodyB->invI;

	// Approximate change in anchors
	// small angle approximation of sin(delta_angle) == delta_angle, cos(delta_angle) == 1
	Vec2 drA = vec2_crossfv(bodyA->deltaAngle, joint->rA);
	Vec2 drB = vec2_crossfv(bodyB->deltaAngle, joint->rB);

	Vec2 rA = vec2_add(joint->rA, drA);
	Vec2 rB = vec2_add(joint->rB, drB);
	Vec2 ds = vec2_add(vec2_sub(bodyB->deltaPosition, bodyA->deltaPosition), vec2_sub(drB, drA));
	Vec2 separation = vec2_add(joint->separation, ds);

	float L = vec2_length(separation);
	Vec2 axis = vec2_norm(separation);

	if (joint->minLength < joint->maxLength)
	{
		if (joint->hertz > 0.0f)
		{
			// Cdot = dot(u, v + cross(w, r))
			Vec2 vr = vec2_add(vec2_sub(vB, vA), vec2_sub(vec2_crossfv(wB, rB), vec2_crossfv(wA, rA)));
			float Cdot = vec2_dot(axis, vr);
			float C = L - joint->length;
			float bias = joint->springBiasCoefficient * C;

			float m = joint->springMassCoefficient * joint->axialMass;
			float impulse = -m * (Cdot + bias) - joint->springImpulseCoefficient * joint->impulse;
			joint->impulse += impulse;

			Vec2 P = vec2_mulfv(impulse, axis);
			vA = vec2_mul_sub(vA, mA,  P);
			wA -= iA * vec2_cross(rA, P);
			vB = vec2_mul_add(vB, mB, P);
			wB += iB * vec2_cross(rB, P);
		}

		// lower limit
		{
			Vec2 vr = vec2_add(vec2_sub(vB, vA), vec2_sub(vec2_crossfv(wB, rB), vec2_crossfv(wA, rA)));
			float Cdot = vec2_dot(axis, vr);

			float C = L - joint->minLength;

			float bias = 0.0f;
			float massScale = 1.0f;
			float impulseScale = 0.0f;
			if (C > 0.0f)
			{
				// speculative
				bias = C * context->inv_dt;
			}
			else if (useBias)
			{
				bias = joint->limitBiasCoefficient * C;
				massScale = joint->limitMassCoefficient;
				impulseScale = joint->limitImpulseCoefficient;
			}

			float impulse = -massScale * joint->axialMass * (Cdot + bias) - impulseScale * joint->lowerImpulse;
			float newImpulse = maxf(0.0f, joint->lowerImpulse + impulse);
			impulse = newImpulse - joint->lowerImpulse;
			joint->lowerImpulse = newImpulse;

			Vec2 P = vec2_mulfv(impulse, axis);
			vA = vec2_mul_sub(vA, mA, P);
			wA -= iA * vec2_cross(rA, P);
			vB = vec2_mul_add(vB, mB, P);
			wB += iB * vec2_cross(rB, P);
		}

		// upper
		{
			Vec2 vr = vec2_add(vec2_sub(vA, vB), vec2_sub(vec2_crossfv(wA, rA), vec2_crossfv(wB, rB)));
			float Cdot = vec2_dot(axis, vr);

			float C = joint->maxLength - L;

			float bias = 0.0f;
			float massScale = 1.0f;
			float impulseScale = 0.0f;
			if (C > 0.0f)
			{
				// speculative
				bias = C * context->inv_dt;
			}
			else if (useBias)
			{
				bias = joint->limitBiasCoefficient * C;
				massScale = joint->limitMassCoefficient;
				impulseScale = joint->limitImpulseCoefficient;
			}

			float impulse = -massScale * joint->axialMass * (Cdot + bias) - impulseScale * joint->upperImpulse;
			float newImpulse = maxf(0.0f, joint->upperImpulse + impulse);
			impulse = newImpulse - joint->upperImpulse;
			joint->upperImpulse = newImpulse;

			Vec2 P = vec2_mulfv(-impulse, axis);
			vA = vec2_mul_sub(vA, mA, P);
			wA -= iA * vec2_cross(rA, P);
			vB = vec2_mul_add(vB, mB, P);
			wB += iB * vec2_cross(rB, P);
		}
	}
	else
	{
		// Equal limits
		Vec2 vr = vec2_add(vec2_sub(vB, vA), vec2_sub(vec2_crossfv(wB, rB), vec2_crossfv(wA, rA)));
		float Cdot = vec2_dot(axis, vr);

		float C = L - joint->minLength;

		float bias = 0.0f;
		float massScale = 1.0f;
		float impulseScale = 0.0f;
		if (useBias)
		{
			bias = joint->limitBiasCoefficient * C;
			massScale = joint->limitMassCoefficient;
			impulseScale = joint->limitImpulseCoefficient;
		}

		float impulse = -massScale * joint->axialMass * (Cdot + bias) - impulseScale * joint->impulse;
		joint->impulse += impulse;

		Vec2 P = vec2_mulfv(impulse, axis);
		vA = vec2_mul_sub(vA, mA, P);
		wA -= iA * vec2_cross(rA, P);
		vB = vec2_mul_add(vB, mB, P);
		wB += iB * vec2_cross(rB, P);
	}

	bodyA->linearVelocity = vA;
	bodyA->angularVelocity = wA;
	bodyB->linearVelocity = vB;
	bodyB->angularVelocity = wB;
}

float b2DistanceJoint_GetConstraintForce(b2JointId jointId, float inverseTimeStep)
{
	b2Joint* base = b2GetJointCheckType(jointId, b2_distanceJoint);
	b2DistanceJoint* joint = &base->distanceJoint;

	return (joint->impulse + joint->lowerImpulse - joint->upperImpulse) * inverseTimeStep;
}

void b2DistanceJoint_SetLength(b2JointId jointId, float length, float minLength, float maxLength)
{
	b2Joint* base = b2GetJointCheckType(jointId, b2_distanceJoint);
	b2DistanceJoint* joint = &base->distanceJoint;

	joint->length = clampf(length, b2_linearSlop, b2_huge);

	minLength = clampf(minLength, b2_linearSlop, b2_huge);
	maxLength = clampf(maxLength, b2_linearSlop, b2_huge);
	joint->minLength = minf(minLength, maxLength);
	joint->maxLength = maxf(minLength, maxLength);

	joint->impulse = 0.0f;
	joint->lowerImpulse = 0.0f;
	joint->upperImpulse = 0.0f;
}

float b2DistanceJoint_GetCurrentLength(b2JointId jointId)
{
	b2Joint* base = b2GetJointCheckType(jointId, b2_distanceJoint);

	b2World* world = b2GetWorldFromIndex(jointId.world);
	
	if (world->locked)
	{
		return 0.0f;
	}

	int32_t indexA = base->edges[0].bodyIndex;
	int32_t indexB = base->edges[1].bodyIndex;
	b2Body* bodyA = world->bodies + indexA;
	b2Body* bodyB = world->bodies + indexB;

	
	

	Vec2 pA = tran2_transform(bodyA->transform, base->localAnchorA);
	Vec2 pB = tran2_transform(bodyB->transform, base->localAnchorB);
	Vec2 d = vec2_sub(pB, pA);
	float length = vec2_length(d);
	return length;
}

void b2DistanceJoint_SetTuning(b2JointId jointId, float hertz, float dampingRatio)
{
	b2Joint* base = b2GetJointCheckType(jointId, b2_distanceJoint);
	b2DistanceJoint* joint = &base->distanceJoint;
	joint->hertz = hertz;
	joint->dampingRatio = dampingRatio;
}

void b2DrawDistance(b2DebugDraw* draw, b2Joint* base, b2Body* bodyA, b2Body* bodyB)
{
	

	b2DistanceJoint* joint = &base->distanceJoint;

	Tran2 xfA = bodyA->transform;
	Tran2 xfB = bodyB->transform;
	Vec2 pA = tran2_transform(xfA, base->localAnchorA);
	Vec2 pB = tran2_transform(xfB, base->localAnchorB);

	Vec2 axis = vec2_norm(vec2_sub(pB, pA));

	Color c1 = {0.7f, 0.7f, 0.7f, 1.0f};
	Color c2 = {0.3f, 0.9f, 0.3f, 1.0f};
	Color c3 = {0.9f, 0.3f, 0.3f, 1.0f};
	Color c4 = {0.4f, 0.4f, 0.4f, 1.0f};

	draw->DrawSegment(pA, pB, c4, draw->context);

	Vec2 pRest = vec2_mul_add(pA, joint->length, axis);
	draw->DrawPoint(pRest, 8.0f, c1, draw->context);

	if (joint->minLength < joint->maxLength)
	{
		Vec2 pMin = vec2_mul_add(pA, joint->minLength, axis);
		Vec2 pMax = vec2_mul_add(pA, joint->maxLength, axis);
		Vec2 offset = vec2_mulfv(0.05f * b2_lengthUnitsPerMeter, vec2_perp_right(axis));

		if (joint->minLength > b2_linearSlop)
		{
			//draw->DrawPoint(pMin, 4.0f, c2, draw->context);
			draw->DrawSegment(vec2_sub(pMin, offset), vec2_add(pMin, offset), c2, draw->context);
		}

		if (joint->maxLength < b2_huge)
		{
			//draw->DrawPoint(pMax, 4.0f, c3, draw->context);
			draw->DrawSegment(vec2_sub(pMax, offset), vec2_add(pMax, offset), c3, draw->context);
		}

		if (joint->minLength > b2_linearSlop && joint->maxLength < b2_huge)
		{
			draw->DrawSegment(pMin, pMax, c4, draw->context);
		}
	}
}
