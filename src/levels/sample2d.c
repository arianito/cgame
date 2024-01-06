#include "graph1.h"

#include <stdlib.h>
#include <string.h>
#include "engine/camera.h"
#include "engine/draw.h"
#include "engine/input.h"
#include "engine/debug.h"
#include "engine/sprite.h"
#include "engine/atlas.h"
#include "mem/alloc.h"
#include "mem/std.h"
#include "math/rot2.h"


#include <stdio.h>

typedef struct
{
} Sample2dContext;

static void create(Sample2dContext *self)
{
}
static int idx = 0;
static Vec3 ps[2];
static void render(Sample2dContext *self)
{
    Ray r = camera_screenToWorld(input->position);
    Vec3 wp = vec3_intersect_plane(r.origin, vec3_mulf(r.direction, 1000), vec3_zero, vec3_forward);

    AABB a = aabb(vec2(0, 0), vec2(30, 30));

    draw_aabb_yz(a, color_red);
    debug_stringf(vec2(10, 40), "%.2f, %.2f\n%.2f, %.2f", a.min.x, a.min.y, a.max.x, a.max.y);
    if (input_mousedown(MOUSE_LEFT))
    {
        if (idx == 2)
            idx = 0;
        ps[idx++] = wp;
    }
    if (idx == 2)
    {
        draw_arrow(ps[0], ps[1], vec3_forward, color_green, 3);
        draw_normal(vec3_zero, vec3yz(rot2_rotate(rot2f(20), vec2(10, 10))), 30, color_green);
    }
    
    
}

static void destroy(Sample2dContext *self)
{
    sprite_clear();
    atlas_clear();
}

Level make_sample2d()
{
    return (Level){
        context : arena_alloc(alloc->global, sizeof(Sample2dContext), sizeof(size_t)),
        create : &create,
        render : &render,
        destroy : &destroy,
    };
}