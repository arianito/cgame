#include "graph1.h"

#include <stdlib.h>
#include <string.h>
#include "engine/mathf.h"
#include "engine/camera.h"
#include "engine/draw.h"
#include "engine/input.h"
#include "engine/debug.h"
#include "engine/noise.h"
#include "engine/sprite.h"
#include "engine/atlas.h"
#include "mem/alloc.h"
#include "mem/std.h"

typedef struct
{
} Sample2dContext;

static void create(Sample2dContext *self)
{
    atlas_load("rocks", "textures/Rocks_source.png");
    int id;
    Sprite *sp;

    id = sprite_create("rocks");
    sp = sprite_get(id);
    id = sprite_create("rocks");
    sp = sprite_get(id);
    sp->rotation.yaw = 45;

}
static void render(Sample2dContext *self)
{
    // Ray r = camera_screenToWorld(input->position);
    // Vec3 wp = vec3_intersectPlane(r.origin, vec3_mulf(r.direction, 1000), vec3_zero, vec3_up);

    // Sprite* sp = sprite_get(0);
    // sp->position = wp;
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