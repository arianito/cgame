#pragma once

#include "engine/draw.h"
#include "engine/debug.h"

#include "math/vec2.h"
#include "math/vec3.h"
#include "math/rot2.h"
#include "math/tran2.h"
#include "math/color.h"

inline static void dbgDrawPolygon(const Vec2 *vertices, int vertexCount, Color color, void *context)
{
    draw_polygon_yz(vertices, vertexCount, color);
}

/// Draw a solid closed polygon provided in CCW order.
inline static void dbgDrawSolidPolygon(const Vec2 *vertices, int vertexCount, Color color, void *context)
{
    fill_polygon_yz(vertices, vertexCount, color_alpha(color, 0.5), false);
    draw_polygon_yz(vertices, vertexCount, color);
}

/// Draw a rounded polygon provided in CCW order.
inline static void dbgDrawRoundedPolygon(const Vec2 *vertices, int vertexCount, float radius, Color lineColor, Color fillColor, void *context)
{
    fill_polygon_yz(vertices, vertexCount, color_alpha(fillColor, 0.5), false);
    draw_polygon_yz(vertices, vertexCount, lineColor);
}

/// Draw a circle.
inline static void dbgDrawCircle(Vec2 center, float radius, Color color, void *context)
{
    draw_circle_yz(vec3yz(center), radius, color, 16);
}

/// Draw a solid circle.
inline static void dbgDrawSolidCircle(Vec2 center, float radius, Vec2 axis, Color color, void *context)
{
    fill_circle_yz(vec3yz(center), radius, color_alpha(color, 0.5), 16, false);
    draw_circle_yz(vec3yz(center), radius, color, 16);
    draw_normal(vec3yz(center), vec3yz(axis), radius, color);
}

/// Draw a capsule.
inline static void dbgDrawCapsule(Vec2 p1, Vec2 p2, float radius, Color color, void *context)
{
    draw_capsule_yz((p1), (p2), radius, color, 16);
}

/// Draw a solid capsule.
inline static void dbgDrawSolidCapsule(Vec2 p1, Vec2 p2, float radius, Color color, void *context)
{
    fill_capsule_yz((p1), (p2), radius, color_alpha(color, 0.5), 16, false);
    draw_capsule_yz((p1), (p2), radius, color, 16);
}

/// Draw a line segment.
inline static void dbgDrawSegment(Vec2 p1, Vec2 p2, Color color, void *context)
{
    draw_line(vec3yz(p1), vec3yz(p2), color);
}

/// Draw a transform. Choose your own length scale.
/// @param xf a transform.
inline static void dbgDrawTransform(Tran2 xf, void *context)
{
    draw_normal(vec3yz(xf.position), vec3yz(rot2_rotate(xf.rotation, vec2_right)), 0.5f, color_red);
    draw_normal(vec3yz(xf.position), vec3yz(rot2_rotate(xf.rotation, vec2_up)),  0.5f, color_red);
}

/// Draw a point.
inline static void dbgDrawPoint(Vec2 p, float size, Color color, void *context)
{
    draw_point(vec3yz(p), size / 10, color);
}

/// Draw a string.
inline static void dbgDrawString(Vec2 p, const char *s, void *context)
{
}