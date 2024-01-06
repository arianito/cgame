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
#include "gui/nuklear.h"

typedef struct
{
} Sample2dContext;

static void create(Sample2dContext *self)
{
    printf("%d\n", sizeof(__m256));
}

static void render(Sample2dContext *self)
{
    Ray r = camera_screenToWorld(input->position);
    Vec3 wp = vec3_intersect_plane(r.origin, vec3_mulf(r.direction, 1000), vec3_zero, vec3_forward);
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