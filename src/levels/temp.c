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
#include "skel/skel.h"
#include "engine/file.h"
#include "engine/editor.h"
#include "engine/grid.h"
#include "math/mat3.h"

typedef struct
{
    int state;
} TempContext;

static void create(TempContext *self)
{
    editor_disable();
    grid_disable();

    camera->position = vec3(-40, 0, 40);
    camera->rotation = rot_look_at(camera->position, vec3(100, 0, 0));
    camera_update();

    atlas_load("spaceship", "textures/spaceship.png");
    mesh_load("spaceship", "models/spaceship.obj");
    atlas_load("bomb", "textures/bomb.png");
    mesh_load("bomb", "models/bomb.obj");

    SpriteId id = sprite_create("spaceship", "spaceship");
    Sprite *sp = sprite_get(id);
    sp->scale = vec3(5, 5, 5);
    sp->position = vec3(0, 0, 0);
    // sprite_crop_pixelart_id(id, 0x021D1010);
}

static float lt = 0;

static void instantiate()
{
    SpriteId id = sprite_create("bomb", "bomb");
    Sprite *sp = sprite_get(id);
    sp->tag = 5;
    sp->scale = vec3(5, 5, 5);
    sp->position = vec3(100, randf() * 60.0f - 30.0f, 0);
}

static void render(TempContext *self)
{

    Sprite *sp = sprite_get(0);
    float ax = input_axis(AXIS_HORIZONTAL);
    ax = ax * ax * signf(ax);
    sp->position.y += ax;
    sp->position.y = clampf(sp->position.y, -30, 30);

    if (input_keydown(KEY_SPACE))
    {
        instantiate();
    }

    sprite_for(it)
    {
        Sprite *sp = it.it;
        if (sp->tag == 5)
        {
            sp->position.x -= gtime->delta * 30.0f;
            if (sp->position.x < -10)
            {
                sprite_delete(sp->id);
                break;
            }
        }
    }
}

static void destroy(TempContext *self)
{
    atlas_clear();
    sprite_clear();
    mesh_clear();

    editor_restore();
    grid_restore();
}

Level make_temp_level()
{
    return (Level){
        context : xxarena(sizeof(TempContext)),
        create : &create,
        render : &render,
        destroy : &destroy,
    };
}