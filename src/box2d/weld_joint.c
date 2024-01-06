// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#include "body.h"
#include "core.h"
#include "joint.h"
#include "solver_data.h"
#include "world.h"

// needed for dll export
#include "box2d/box2d.h"
#include "box2d/debug_draw.h"

// Point-to-point constraint
// C = p2 - p1
// Cdot = v2 - v1
//      = v2 + cross(w2, r2) - v1 - cross(w1, r1)
// J = [-I -r1_skew I r2_skew ]
// Identity used:
// w k % (rx i + ry j) = w * (-ry i + rx j)

// Angle constraint
// C = angle2 - angle1 - referenceAngle
// Cdot = w2 - w1
// J = [0 0 -1 0 0 1]
// K = invI1 + invI2

void b2PrepareWeldJoint(b2Joint* base, b2StepContext* context)
{
	

	int32_t indexA = base->edges[0].bodyIndex;
	int32_t indexB = base->edges[1].bodyIndex;
	
	

	b2Body* bodyA = context->bodies + indexA;
	b2Body* bodyB = context->bodies + indexB;
	
	

	float mA = bodyA->invMass;
	float iA = bodyA->invI;
	float mB = bodyB->invMass;
	float iB = bodyB->invI;

	b2WeldJoint* joint = &base->weldJoint;
	joint->indexA = context->bodyToSolverMap[indexA];
	joint->indexB = context->bodyToSolverMap[indexB];

	joint->rA = rot2_rotate(bodyA->transform.rotation, vec2_sub(base->localAnchorA, bodyA->localCenter));
	joint->rB = rot2_rotate(bodyB->transform.rotation, vec2_sub(base->localAnchorB, bodyB->localCenter));
	joint->linearSeparation = vec2_add(vec2_sub(joint->rB, joint->rA), vec2_sub(bodyB->position, bodyA->position));
	joint->angularSeparation = bodyB->angle - bodyA->angle - joint->referenceAngle;

	Vec2 rA = joint->rA;
	Vec2 rB = joint->rB;

	// TODO_ERIN linear and angular coupling leads to instabilities and poor behavior
	Mat2 K;
	K.cx.x = mA + mB + rA.y * rA.y * iA + rB.y * rB.y * iB;
	K.cx.y = -rA.y * rA.x * iA - rB.y * rB.x * iB;
	K.cy.x = K.cx.y;
	K.cy.y = mA + mB + rA.x * rA.x * iA + rB.x * rB.x * iB;
	joint->pivotMass = mat2_inv(K);

	float Ka = iA + iB;
	joint->axialMass = Ka > 0.0f ? 1.0f / Ka : 0.0f;

	const float h = context->dt;

	float linearHertz = joint->linearHertz;
	if (linearHertz == 0.0f)
	{
		linearHertz = 0.25f * context->velocityIterations * context->inv_dt;

		// no warm staring
		joint->linearImpulse = vec2_zero;
	}
	else
	{
		joint->linearImpulse = vec2_mulfv(context->dtRatio, joint->linearImpulse);
	}

	{
		const float zeta = joint->linearDampingRatio;
		const float omega = 2.0f * b2_pi * linearHertz;
		joint->linearBiasCoefficient = omega / (2.0f * zeta + h * omega);
		float a = h * omega * (2.0f * zeta + h * omega);
		joint->linearImpulseCoefficient = 1.0f / (1.0f + a);
		joint->linearMassCoefficient = a * joint->linearImpulseCoefficient;
	}

	float angularHertz = joint->angularHertz;
	if (angularHertz == 0.0f)
	{
		angularHertz = 0.25f * context->velocityIterations * context->inv_dt;

		// no warm staring
		joint->angularImpulse = 0.0f;
	}
	else
	{
		joint->angularImpulse = context->dtRatio * joint->angularImpulse;
	}

	{
		const float zeta = joint->angularDampingRatio;
		const float omega = 2.0f * b2_pi * angularHertz;
		joint->angularBiasCoefficient = omega / (2.0f * zeta + h * omega);
		float a = h * omega * (2.0f * zeta + h * omega);
		joint->angularImpulseCoefficient = 1.0f / (1.0f + a);
		joint->angularMassCoefficient = a * joint->angularImpulseCoefficient;
	}

	if (context->enableWarmStarting)
	{
		float dtRatio = context->dtRatio;

		// Soft step works best when bilateral constraints have no warm starting.
		joint->linearImpulse.x = dtRatio;
		joint->linearImpulse.y = dtRatio;
		joint->angularImpulse *= dtRatio;
	}
	else
	{
		joint->linearImpulse = vec2_zero;
		joint->angularImpulse = 0.0f;
	}
}

void b2WarmStartWeldJoint(b2Joint* base, b2StepContext* context)
{
	b2WeldJoint* joint = &base->weldJoint;

	b2SolverBody* bodyA = context->solverBodies + joint->indexA;
	Vec2 vA = bodyA->linearVelocity;
	float wA = bodyA->angularVelocity;
	float mA = bodyA->invMass;
	float iA = bodyA->invI;

	b2SolverBody* bodyB = context->solverBodies + joint->indexB;
	Vec2 vB = bodyB->linearVelocity;
	float wB = bodyB->angularVelocity;
	float mB = bodyB->invMass;
	float iB = bodyB->invI;

	vA = vec2_mul_sub(vA, mA, joint->linearImpulse);
	wA -= iA * (vec2_cross(joint->rA, joint->linearImpulse) + joint->angularImpulse);

	vB = vec2_mul_add(vB, mB, joint->linearImpulse);
	wB += iB * (vec2_cross(joint->rB, joint->linearImpulse) + joint->angularImpulse);

	bodyA->linearVelocity = vA;
	bodyA->angularVelocity = wA;
	bodyB->linearVelocity = vB;
	bodyB->angularVelocity = wB;
}

void b2SolveWeldJoint(b2Joint* base, const b2StepContext* context, bool useBias)
{
	

	b2WeldJoint* joint = &base->weldJoint;

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

	Vec2 linearBias = vec2_zero;
	float angularBias = 0.0f;

	float linearMassScale = 1.0f;
	float linearImpulseScale = 0.0f;
	float angularMassScale = 1.0f;
	float angularImpulseScale = 0.0f;
	if (useBias)
	{
		Vec2 ds = vec2_add(vec2_sub(bodyB->deltaPosition, bodyA->deltaPosition), vec2_sub(drB, drA));
		Vec2 linearSeparation = vec2_add(joint->linearSeparation, ds);
		linearBias = vec2_mulfv(joint->linearBiasCoefficient, linearSeparation);

		float angularSeperation = joint->angularSeparation + bodyB->deltaAngle - bodyA->deltaAngle;
		angularBias = joint->angularBiasCoefficient * angularSeperation;

		linearMassScale = joint->linearMassCoefficient;
		linearImpulseScale = joint->linearImpulseCoefficient;
		angularMassScale = joint->angularMassCoefficient;
		angularImpulseScale = joint->angularImpulseCoefficient;
	}

	// Note: don't relax user softness

	// Axial constraint
	if (useBias || joint->angularHertz == 0.0f)
	{
		float Cdot = wB - wA;
		float b = joint->axialMass * (Cdot + angularBias);
		float impulse = -angularMassScale * b - angularImpulseScale * joint->angularImpulse;
		joint->angularImpulse += impulse;
		wA -= iA * impulse;
		wB += iB * impulse;
	}

	// Linear constraint
	if (useBias || joint->linearHertz == 0.0f)
	{
		Vec2 Cdot = vec2_sub(vec2_add(vB, vec2_crossfv(wB, rB)), vec2_add(vA, vec2_crossfv(wA, rA)));
		Vec2 b = mat2_mulv(joint->pivotMass, vec2_add(Cdot, linearBias));

		Vec2 impulse = {
			-linearMassScale * b.x - linearImpulseScale * joint->linearImpulse.x,
			-linearMassScale * b.y - linearImpulseScale * joint->linearImpulse.y,
		};

		joint->linearImpulse = vec2_add(joint->linearImpulse, impulse);

		vA = vec2_mul_sub(vA, mA, impulse);
		wA -= iA * vec2_cross(rA, impulse);

		vB = vec2_mul_add(vB, mB, impulse);
		wB += iB * vec2_cross(rB, impulse);
	}

	bodyA->linearVelocity = vA;
	bodyA->angularVelocity = wA;
	bodyB->linearVelocity = vB;
	bodyB->angularVelocity = wB;
}

#if 0
void b2DumpWeldJoint()
{
	int32 indexA = m_bodyA->m_islandIndex;
	int32 indexB = m_bodyB->m_islandIndex;

	b2Dump("  b2WeldJointDef jd;\n");
	b2Dump("  jd.bodyA = bodies[%d];\n", indexA);
	b2Dump("  jd.bodyB = bodies[%d];\n", indexB);
	b2Dump("  jd.collideConnected = bool(%d);\n", m_collideConnected);
	b2Dump("  jd.localAnchorA.Set(%.9g, %.9g);\n", m_localAnchorA.x, m_localAnchorA.y);
	b2Dump("  jd.localAnchorB.Set(%.9g, %.9g);\n", m_localAnchorB.x, m_localAnchorB.y);
	b2Dump("  jd.referenceAngle = %.9g;\n", m_referenceAngle);
	b2Dump("  jd.stiffness = %.9g;\n", m_stiffness);
	b2Dump("  jd.damping = %.9g;\n", m_damping);
	b2Dump("  joints[%d] = m_world->CreateJoint(&jd);\n", m_index);
}
#endif
