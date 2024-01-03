#include "graph1.h"

#include <stdlib.h>
#include <string.h>
#include "engine/mathf.h"
#include "engine/camera.h"
#include "engine/draw.h"
#include "engine/input.h"
#include "engine/debug.h"
#include "engine/noise.h"
#include "engine/atlas.h"
#include "engine/sprite.h"
#include "mem/alloc.h"
#include "mem/std.h"

typedef struct
{
} Sample2dContext;

static void create(Sample2dContext *self)
{
    atlas_init();
    sprite_init();
}

static void update(Sample2dContext *self)
{
    sprite_render();
}

static void destroy(Sample2dContext *self)
{
    sprite_destroy();
    atlas_destroy();
}

Level make_sample2d()
{
    return (Level){
        context : arena_alloc(alloc->global, sizeof(Sample2dContext), sizeof(size_t)),
        create : &create,
        update : &update,
        destroy : &destroy,
    };
}