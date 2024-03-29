// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#include "pool.h"

#include "box2d/joint_types.h"

#include <stdint.h>
#include "math/mat2.h"

typedef struct b2DebugDraw b2DebugDraw;
typedef struct b2SolverTaskContext b2SolverTaskContext;
typedef struct b2StepContext b2StepContext;
typedef struct b2World b2World;

/// A joint edge is used to connect bodies and joints together
/// in a joint graph where each body is a node and each joint
/// is an edge. A joint edge belongs to a doubly linked list
/// maintained in each attached body. Each joint has two joint
/// nodes, one for each attached body.
typedef struct b2JointEdge
{
	int32_t bodyIndex;
	int32_t prevKey;
	int32_t nextKey;
} b2JointEdge;

typedef struct b2DistanceJoint
{
	float hertz;
	float dampingRatio;
	float length;
	float minLength;
	float maxLength;

	// Solver shared
	float impulse;
	float lowerImpulse;
	float upperImpulse;

	// Solver temp
	int32_t indexA;
	int32_t indexB;
	Vec2 rA;
	Vec2 rB;
	Vec2 separation;
	float springBiasCoefficient;
	float springMassCoefficient;
	float springImpulseCoefficient;
	float limitBiasCoefficient;
	float limitMassCoefficient;
	float limitImpulseCoefficient;
	float axialMass;
} b2DistanceJoint;

typedef struct b2MotorJoint
{
	// Solver shared
	Vec2 linearOffset;
	float angularOffset;
	Vec2 linearImpulse;
	float angularImpulse;
	float maxForce;
	float maxTorque;
	float correctionFactor;

	// Solver temp
	int32_t indexA;
	int32_t indexB;
	Vec2 rA;
	Vec2 rB;
	Vec2 linearSeparation;
	float angularSeparation;
	Mat2 linearMass;
	float angularMass;
} b2MotorJoint;

typedef struct b2MouseJoint
{
	Vec2 targetA;
	float stiffness;
	float damping;
	float beta;

	// Solver shared
	Vec2 impulse;
	float maxForce;
	float gamma;

	// Solver temp
	int32_t indexB;
	Vec2 positionB;
	Vec2 rB;
	Vec2 localCenterB;
	Mat2 mass;
	Vec2 C;
} b2MouseJoint;

typedef struct b2PrismaticJoint
{
	// Solver shared
	Vec2 localAxisA;
	Vec2 impulse;
	float motorImpulse;
	float lowerImpulse;
	float upperImpulse;
	bool enableMotor;
	float maxMotorForce;
	float motorSpeed;
	bool enableLimit;
	float referenceAngle;
	float lowerTranslation;
	float upperTranslation;

	// Solver temp
	int32_t indexA;
	int32_t indexB;
	Vec2 rA;
	Vec2 rB;
	Vec2 axisA;
	Vec2 pivotSeparation;
	float angleSeparation;
	Mat2 pivotMass;
	float axialMass;
	float biasCoefficient;
	float massCoefficient;
	float impulseCoefficient;
} b2PrismaticJoint;

typedef struct b2RevoluteJoint
{
	// Solver shared
	Vec2 linearImpulse;
	float motorImpulse;
	float lowerImpulse;
	float upperImpulse;
	bool enableMotor;
	float maxMotorTorque;
	float motorSpeed;
	bool enableLimit;
	float referenceAngle;
	float lowerAngle;
	float upperAngle;

	// Solver temp
	int32_t indexA;
	int32_t indexB;
	float angleA;
	float angleB;
	Vec2 rA;
	Vec2 rB;
	Vec2 separation;
	Mat2 pivotMass;
	float limitBiasCoefficient;
	float limitMassCoefficient;
	float limitImpulseCoefficient;
	float biasCoefficient;
	float massCoefficient;
	float impulseCoefficient;
	float axialMass;
} b2RevoluteJoint;

typedef struct b2WeldJoint
{
	// Solver shared
	float referenceAngle;
	float linearHertz;
	float linearDampingRatio;
	float angularHertz;
	float angularDampingRatio;
	float linearBiasCoefficient;
	float linearMassCoefficient;
	float linearImpulseCoefficient;
	float angularBiasCoefficient;
	float angularMassCoefficient;
	float angularImpulseCoefficient;
	Vec2 linearImpulse;
	float angularImpulse;

	// Solver temp
	int32_t indexA;
	int32_t indexB;
	Vec2 rA;
	Vec2 rB;
	Vec2 linearSeparation;
	float angularSeparation;
	Mat2 pivotMass;
	float axialMass;
} b2WeldJoint;

typedef struct b2WheelJoint
{
	// Solver shared
	Vec2 localAxisA;
	float perpImpulse;
	float motorImpulse;
	float springImpulse;
	float lowerImpulse;
	float upperImpulse;
	float maxMotorTorque;
	float motorSpeed;
	float lowerTranslation;
	float upperTranslation;
	float stiffness;
	float damping;
	bool enableMotor;
	bool enableLimit;

	// Solver temp
	int32_t indexA;
	int32_t indexB;
	Vec2 rA;
	Vec2 rB;
	Vec2 axisA;
	Vec2 pivotSeparation;
	float perpMass;
	float motorMass;
	float axialMass;
	float springMass;
	float bias;
	float gamma;
	float biasCoefficient;
	float massCoefficient;
	float impulseCoefficient;
} b2WheelJoint;

/// The base joint class. Joints are used to constraint two bodies together in
/// various fashions. Some joints also feature limits and motors.
typedef struct b2Joint
{
	b2Object object;
	b2JointType type;
	b2JointEdge edges[2];

	int32_t islandIndex;
	int32_t islandPrev;
	int32_t islandNext;

	// The color of this constraint in the graph coloring
	int32_t colorIndex;

	// Index of joint within color
	int32_t colorSubIndex;

	Vec2 localAnchorA;
	Vec2 localAnchorB;

	union
	{
		b2DistanceJoint distanceJoint;
		b2MotorJoint motorJoint;
		b2MouseJoint mouseJoint;
		b2RevoluteJoint revoluteJoint;
		b2PrismaticJoint prismaticJoint;
		b2WeldJoint weldJoint;
		b2WheelJoint wheelJoint;
	};

	float drawSize;
	bool isMarked;
	bool collideConnected;
} b2Joint;

b2Joint* b2GetJoint(b2World* world, b2JointId jointId);

// todo remove this
b2Joint* b2GetJointCheckType(b2JointId id, b2JointType type);

void b2PrepareJoint(b2Joint* joint, b2StepContext* context);
void b2WarmStartJoint(b2Joint* joint, b2StepContext* context);
void b2SolveJoint(b2Joint* joint, b2StepContext* context, bool useBias);

void b2PrepareAndWarmStartOverflowJoints(b2SolverTaskContext* context);
void b2SolveOverflowJoints(b2SolverTaskContext* context, bool useBias);

void b2DrawJoint(b2DebugDraw* draw, b2World* world, b2Joint* joint);
