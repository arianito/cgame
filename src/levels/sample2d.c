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
} Sample2dContext;

static void create(Sample2dContext *self)
{

    atlas_load("platform", "textures/textures.png");
    mesh_load("box", "models/box.obj");
    mesh_load("plane", "models/plane.obj");

    {
        SpriteId id = sprite_create("plane","platform");
        Sprite *sp = sprite_get(id);
        sp->scale = vec3(5, 5, 5);
        sp->position = vec3(0, 0, 10);
        sprite_crop_pixelart_id(id, 0x021D1010);
    }
    {
        SpriteId id = sprite_create("box", "platform");
        Sprite *sp = sprite_get(id);
        sp->scale = vec3(5, 5, 5);
        sp->position = vec3(0, 0, 0);
        sprite_crop_pixelart_id(id, 0x021D1010);
    }
}
static void render(Sample2dContext *self)
{
}

static void destroy(Sample2dContext *self)
{
    atlas_clear();
    sprite_clear();
    mesh_clear();
}

Level make_sample2d()
{
    return (Level){
        context : xxarena(sizeof(Sample2dContext)),
        create : &create,
        render : &render,
        destroy : &destroy,
    };
}