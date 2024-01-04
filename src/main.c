
#include <stdio.h>
#include "mem/alloc.h"

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
#include "engine/sprite.h"

#include "levels/graph1.h"
#include "levels/sample2d.h"

int main(int argc, char **argv)
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
    sprite_init();

    level_add(make_sample2d());
    level_add(make_graph1());
    level_activate(0);

    int debug = 1;
    while (game_loop())
    {
        input_update();
        editor_update();
        grid_render();

        if (input_keydown(KEY_TAB))
            debug ^= 1;
        else if (input_keydown(KEY_RIGHT_BRACKET))
            level_next();
        else if (input_keydown(KEY_LEFT_BRACKET))
            level_prev();

        level_render();

        sprite_render();

        atlas_update();

        if (debug)
        {
            debug_origin(vec2(0, 1));
            debug_color(color_yellow);
            debug_rotation(rot_zero);
            debug_stringf(vec2(10, game->height - 10), "global: %d / %d\nstack: %d / %d\nmemory: %d",
                          alloc->global->usage, alloc->global->total,
                          alloc->stack->usage, alloc->stack->total, alloc->usage);
        }
        draw_render();
        debug_render();
    }

    level_destroy();
    sprite_destroy();
    atlas_destroy();
    debug_terminate();
    grid_terminate();
    draw_terminate();
    game_terminate();
    alloc_terminate();
    return 0;
}