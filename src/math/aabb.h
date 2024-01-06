#ifndef cgame_AABB_H
#define cgame_AABB_H

#include <stdbool.h>
#include "math/vec2.h"

#define AABB_MARGIN 0.1

typedef struct
{
    Vec2 min;
    Vec2 max;
} AABB;

#define aabb(a, b) ((AABB){a, b})

static inline bool aabb_valid(AABB a)
{
    Vec2 d = vec2_sub(a.max, a.min);
    bool valid = d.x >= 0.0f && d.y >= 0.0f;
    valid = valid && vec2_valid(a.min) && vec2_valid(a.max);
    return valid;
}

static inline float aabb_perimeter(AABB a)
{
    float wx = a.max.x - a.min.x;
    float wy = a.max.y - a.min.y;
    return 2.0f * (wx + wy);
}

static inline AABB aabb_extend(AABB a)
{
    AABB c;
    c.min.x = a.min.x - AABB_MARGIN;
    c.min.y = a.min.y - AABB_MARGIN;
    c.max.x = a.max.x + AABB_MARGIN;
    c.max.y = a.max.y + AABB_MARGIN;
    return c;
}

static inline bool aabb_enlarge(AABB *a, AABB b)
{
    bool changed = false;
    if (b.min.x < a->min.x)
    {
        a->min.x = b.min.x;
        changed = true;
    }

    if (b.min.y < a->min.y)
    {
        a->min.y = b.min.y;
        changed = true;
    }

    if (a->max.x < b.max.x)
    {
        a->max.x = b.max.x;
        changed = true;
    }

    if (a->max.y < b.max.y)
    {
        a->max.y = b.max.y;
        changed = true;
    }

    return changed;
}

static inline bool aabb_contains_with_margin(AABB a, AABB b, float margin)
{
    return (a.min.x <= b.min.x - margin) & (a.min.y <= b.min.y - margin) &
           (b.max.x + margin <= a.max.x) & (b.max.y + margin <= a.max.y);
}

static inline bool aabb_overlaps(AABB a, AABB b)
{
    Vec2 d1 = vec2(b.min.x - a.max.x, b.min.y - a.max.y);
    Vec2 d2 = vec2(a.min.x - b.max.x, a.min.y - b.max.y);

    if (d1.x > 0.0f || d1.y > 0.0f)
        return false;

    if (d2.x > 0.0f || d2.y > 0.0f)
        return false;

    return true;
}


static inline bool aabb_contains(AABB a, AABB b)
{
	bool s = true;
	s = s && a.min.x <= b.min.x;
	s = s && a.min.y <= b.min.y;
	s = s && b.max.x <= a.max.x;
	s = s && b.max.y <= a.max.y;
	return s;
}

/// Get the center of the AABB.
static inline Vec2 aabb_center(AABB a)
{
	return vec2(0.5f * (a.min.x + a.max.x), 0.5f * (a.min.y + a.max.y));
}

/// Get the extents of the AABB (half-widths).
static inline Vec2 aabb_extents(AABB a)
{
	return vec2(0.5f * (a.max.x - a.min.x), 0.5f * (a.max.y - a.min.y));
}

/// Union of two AABBs
static inline AABB aabb_union(AABB a, AABB b)
{
	AABB c;
	c.min.x = minf(a.min.x, b.min.x);
	c.min.y = minf(a.min.y, b.min.y);
	c.max.x = maxf(a.max.x, b.max.x);
	c.max.y = maxf(a.max.y, b.max.y);
	return c;
}

#endif