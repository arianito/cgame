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
#include "mem/alloc.h"
#include "math/rot2.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "gui/cimgui.h"

typedef struct
{
} Sample2dContext;

static void create(Sample2dContext *self)
{

}

static void* ptr = NULL;
static void render(Sample2dContext *self)
{
    Ray r = camera_screenToWorld(input->position);
    Vec3 wp = vec3_intersect_plane(r.origin, vec3_mulf(r.direction, 1000), vec3_zero, vec3_forward);
    draw_capsule_yz(vec3_zero, wp, 10, color_red, 8);

    if(input_keydown(KEY_H)) {
        ptr = stack_alloc(alloc->stack, 1000);
    }
    if(input_keydown(KEY_K)) {
        ptr = stack_realloc(alloc->stack, ptr, 1000);
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
        context : arena_alloc(alloc->global, sizeof(Sample2dContext)),
        create : &create,
        render : &render,
        destroy : &destroy,
    };
}