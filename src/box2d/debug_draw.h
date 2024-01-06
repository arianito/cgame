// SPDX-FileCopyrightText: 2022 Erin Catto
// SPDX-License-Identifier: MIT

#pragma once

#include "types.h"
#include "math/vec2.h"

/// This struct holds callbacks you can implement to draw a box2d world.
typedef struct b2DebugDraw
{
	/// Draw a closed polygon provided in CCW order.
	void (*DrawPolygon)(const Vec2* vertices, int vertexCount, b2Color color, void* context);

	/// Draw a solid closed polygon provided in CCW order.
	void (*DrawSolidPolygon)(const Vec2* vertices, int vertexCount, b2Color color, void* context);

	/// Draw a rounded polygon provided in CCW order.
	void (*DrawRoundedPolygon)(const Vec2* vertices, int vertexCount, float radius, b2Color lineColor, b2Color fillColor, void* context);

	/// Draw a circle.
	void (*DrawCircle)(Vec2 center, float radius, b2Color color, void* context);

	/// Draw a solid circle.
	void (*DrawSolidCircle)(Vec2 center, float radius, Vec2 axis, b2Color color, void* context);

	/// Draw a capsule.
	void (*DrawCapsule)(Vec2 p1, Vec2 p2, float radius, b2Color color, void* context);

	/// Draw a solid capsule.
	void (*DrawSolidCapsule)(Vec2 p1, Vec2 p2, float radius, b2Color color, void* context);

	/// Draw a line segment.
	void (*DrawSegment)(Vec2 p1, Vec2 p2, b2Color color, void* context);

	/// Draw a transform. Choose your own length scale.
	/// @param xf a transform.
	void (*DrawTransform)(Tran2 xf, void* context);

	/// Draw a point.
	void (*DrawPoint)(Vec2 p, float size, b2Color color, void* context);

	/// Draw a string.
	void (*DrawString)(Vec2 p, const char* s, void* context);

	bool drawShapes;
	bool drawJoints;
	bool drawAABBs;
	bool drawMass;
	bool drawContacts;
	bool drawGraphColors;
	bool drawContactNormals;
	bool drawContactImpulses;
	bool drawFrictionImpulses;
	void* context;
} b2DebugDraw;
