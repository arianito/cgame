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
        seq->type = 0;
        seq->name = str("bone x");
        seq->length = 4;
        seq->frames = xxmalloc(sizeof(KeyFrame) * 4);
        seq->frames[0] = (KeyFrame){0, 0};
        seq->frames[1] = (KeyFrame){1, 2.5};
        seq->frames[2] = (KeyFrame){3, 4};
        seq->frames[3] = (KeyFrame){6, 0};
    }
    {
        AnimSequence *seq = &sequences[1];
        seq->id = 1;
        seq->i0 = 0;
        seq->type = 0;
        seq->name = str("bone rotation");
        seq->length = 3;
        seq->frames = xxmalloc(sizeof(KeyFrame) * 3);
        seq->frames[0] = (KeyFrame){0, 1};
        seq->frames[1] = (KeyFrame){2, 4};
        seq->frames[2] = (KeyFrame){5, 3};
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

    igSequencer("hello", 400, &ctx);

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