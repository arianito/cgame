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

typedef struct
{
} Sample2dContext;

static void create(Sample2dContext *self)
{

    atlas_load("platform", "textures/textures.png");
    gui_init(game->window);

    SpriteId id = sprite_create("platform");
    Sprite *sp = sprite_get(id);
    sp->scale = vec2(10, 10);
    // sp->rotation = rot(90, 0, 0);
    sp->material.cropped_area = rect(0, 0, 16, 16);
    sprite_crop_pixelart_id(id, 0x021D1010);

    mesh_from_obj("models/box.obj");
}
static int j = 0;
static void render(Sample2dContext *self)
{

    gui_begin();

    igBegin("Hello, world!", NULL, 0);
    igText("This is some useful text");
    if (igButton("click on me", (ImVec2){100, 20}))
    {
        j++;
        sprite_crop_pixelart(0, vec2(j % 16, j / 16), vec2(16, 16));
    }
    igEnd();

    gui_end();
}

static void destroy(Sample2dContext *self)
{
    gui_destroy();
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