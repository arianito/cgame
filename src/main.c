
#include <stdio.h>

#include "mem/alloc.h"
#include "mem/utils.h"

#include "engine/game.h"

#include "engine/draw.h"
#include "engine/grid.h"
#include "engine/camera.h"
#include "engine/editor.h"
#include "engine/debug.h"
#include "engine/input.h"
#include "engine/file.h"
#include "engine/level.h"
#include "engine/atlas.h"
#include "engine/mesh.h"
#include "engine/sprite.h"

#include "levels/graph1.h"
#include "levels/sample2d.h"
#include "levels/box2d_sample.h"
#include "levels/skeleton_test.h"

int main()
{
    MemoryMetadata meta;
    meta.global = 8 * MEGABYTES;
    meta.stack = 1 * MEGABYTES;
    alloc_create(meta);

    file_init("../assets/");
    game_init();
    input_init();
    camera_init();
    draw_init();
    editor_init();
    grid_init();
    debug_init();
    level_init(8);
    atlas_init();
    mesh_init();
    sprite_init();

    level_add(make_graph1());
    level_add(make_skeleton_testbed());
    level_add(make_sample2d());
    level_add(make_box2dsample());
    level_activate(0);

    int debug = 1;
    while (true)
    {
        game_begin();
        input_begin();
        level_render_before();

        grid_render();
        editor_update();

        if (input_keydown(KEY_TAB))
            debug ^= 1;
        else if (input_keydown(KEY_RIGHT_BRACKET))
            level_next();
        else if (input_keydown(KEY_LEFT_BRACKET))
            level_prev();

        level_render();
        sprite_render();

        if (debug)
        {
            debug_origin(vec2(0, 1));
            debug_color(color_yellow);
            debug_rotation(rot_zero);
            debug_stringf(vec2(10, game->size.y - 10), "global: %d / %d\nstack: %d / %d\nmemory: %d",
                          alloc->global->usage, alloc->global->total,
                          alloc->stack->usage, alloc->stack->total, alloc->usage);
        }
        draw_render();
        debug_render();

        input_end();
        level_render_after();

        if (!game_end())
            break;
        
    }

    level_destroy();
    sprite_destroy();
    mesh_destroy();
    atlas_destroy();
    debug_terminate();
    grid_terminate();
    draw_terminate();
    game_terminate();
    alloc_terminate();
    return 0;
}