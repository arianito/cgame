#include "graph1.h"

#include <stdio.h>
#include <stdlib.h>

#include "gui/libgui.h"
#include "engine/camera.h"
#include "engine/draw.h"
#include "engine/input.h"
#include "engine/debug.h"
#include "engine/sprite.h"
#include "engine/atlas.h"
#include "engine/mesh.h"
#include "mem/alloc.h"
#include "math/rot2.h"
#include "math/noise.h"

typedef struct
{
} SkeletonTestbestContext;

static void create(SkeletonTestbestContext *self)
{

    atlas_load("platform", "textures/textures.png");
    mesh_load("bone", "models/plane.obj");

    gui_init(game->window);
    {
        SpriteId id = sprite_create("bone","platform");
        Sprite *sp = sprite_get(id);
        sp->scale = vec3(10, 10, 10);
        sp->position = vec3(0, 0, 0);
        sprite_crop_pixelart_id(id, 0x021D1010);
    }
}
static void render(SkeletonTestbestContext *self)
{
}

static void destroy(SkeletonTestbestContext *self)
{
    atlas_clear();
    sprite_clear();
    mesh_clear();
    gui_destroy();
}

Level make_skeleton_testbed()
{
    return (Level){
        context : xxarena(sizeof(SkeletonTestbestContext)),
        create : &create,
        render : &render,
        destroy : &destroy,
    };
}