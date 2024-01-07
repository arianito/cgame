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
#include "mem/alloc.h"
#include "math/rot2.h"

typedef struct
{
} Sample2dContext;

static void create(Sample2dContext *self)
{
    gui_init(game->window);
}

static void render(Sample2dContext *self)
{

    gui_begin();
    
    igBegin("Hello, world!", NULL, 0);
    igText("This is some useful text");
    if(igButton("click on me", (ImVec2){100, 20})) {
        printf("ok\n");
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