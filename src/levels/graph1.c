#include "graph1.h"

#include <stdlib.h>
#include <string.h>
#include "engine/camera.h"
#include "engine/draw.h"
#include "engine/input.h"
#include "engine/debug.h"
#include "mem/alloc.h"

#include "skel/anim.h"
#include "skel/anim_editor.h"

#include "gui/libgui.h"

typedef struct
{
} Graph1Context;

static AnimSequenceContext ctx = {0};

static void create(Graph1Context *self)
{
    gui_init(NULL);

    camera->offset = 0.4f;

    AnimSequence *sequences = xxmalloc(sizeof(AnimSequence) * 2);
    {
        AnimSequence *seq = &sequences[0];
        seq->id = 0;
        seq->i0 = 0;
        seq->state0 = 0;
        seq->scale0 = 1;
        seq->type = 0;
        seq->name = str("bone x pos");
        seq->length = 4;
        seq->frames = xxmalloc(sizeof(KeyFrame) * 4);
        seq->frames[0] = (KeyFrame){0, 30, {0}};
        seq->frames[1] = (KeyFrame){1, -10, {0}};
        seq->frames[2] = (KeyFrame){3, 5, {0}};
        seq->frames[3] = (KeyFrame){6, -2, {0}};
    }
    {
        AnimSequence *seq = &sequences[1];
        seq->id = 1;
        seq->i0 = 0;
        seq->state0 = 0;
        seq->scale0 = 1;
        seq->type = 0;
        seq->name = str("bone rotation x");
        seq->length = 3;
        seq->frames = xxmalloc(sizeof(KeyFrame) * 3);
        seq->frames[0] = (KeyFrame){0, 0, {.49,.15,.5,.82}};
        seq->frames[1] = (KeyFrame){1, 90, {.49,.15,.5,.82}};
        seq->frames[2] = (KeyFrame){6, 270, {.49,.15,.5,.82}};
    }

    Anim *anim = xxmalloc(sizeof(Anim) * 1);
    anim->name = str("walk");
    anim->data = sequences;
    anim->length = 2;

    ctx.time = 0;
    ctx.anim = anim;
}

static void render(Graph1Context *self)
{
    camera_update();
}

static void render_after(Graph1Context *self)
{
    gui_begin();

    igSequencer("hello", 600, &ctx);

    float x = anim_iterpolate(&ctx.anim->data[0], ctx.time);
    float r = anim_iterpolate(&ctx.anim->data[1], ctx.time);

    draw_axisRot(vec3(0, 0, x), 10, rot(0, 0, r));

    gui_end();
}
static void destroy(Graph1Context *self)
{
    gui_destroy();
}

Level make_graph1()
{
    return (Level){
        context : xxarena(sizeof(Graph1Context)),
        create : &create,
        render : &render,
        render_after : &render_after,
        destroy : &destroy,
    };
}