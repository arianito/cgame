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
#include "skel/skel.h"

typedef struct
{
} Graph1Context;

static AnimSequenceContext ctx = {0};
static Skel* skel;

static void create(Graph1Context *self)
{
    gui_init(NULL);

    camera->offset = 0.25f;
    camera_update();

    AnimSequence *sequences = xxmalloc(sizeof(AnimSequence) * 3);
    {
        AnimSequence *seq = &sequences[0];
        seq->id = 0;
        seq->i0 = 0;
        seq->type = ANIM_PROP_X;
        seq->name = anim_property_names[seq->type];
        seq->length = 4;
        seq->frames = xxmalloc(sizeof(KeyFrame) * 4);
        seq->frames[0] = (KeyFrame){0, 10, {-0.2, -2, 0.5, 5}};
        seq->frames[1] = (KeyFrame){1, -5, {-0.2, -2, 0.5, 5}};
        seq->frames[2] = (KeyFrame){3, 20, {-0.2, -2, 0.5, 5}};
        seq->frames[3] = (KeyFrame){6, 10, {-0.2, -2, 0.5, 5}};
    }
    {
        AnimSequence *seq = &sequences[1];
        seq->id = 0;
        seq->i0 = 0;
        seq->type = ANIM_PROP_Y;
        seq->name = anim_property_names[seq->type];
        seq->length = 4;
        seq->frames = xxmalloc(sizeof(KeyFrame) * 4);
        seq->frames[0] = (KeyFrame){0, -10, {-0.2, -2, 0.5, 5}};
        seq->frames[1] = (KeyFrame){2, -15, {-0.2, -2, 0.5, 5}};
        seq->frames[2] = (KeyFrame){4, -20, {-0.2, -2, 0.5, 5}};
        seq->frames[3] = (KeyFrame){7, -10, {-0.2, -2, 0.5, 5}};
    }
    {
        AnimSequence *seq = &sequences[2];
        seq->id = 1;
        seq->i0 = 0;
        seq->type = ANIM_PROP_ROTATION;
        seq->name = anim_property_names[seq->type];
        seq->length = 3;
        seq->frames = xxmalloc(sizeof(KeyFrame) * 3);
        seq->frames[0] = (KeyFrame){0, 0, {-0.2, -2, 0.5, 5}};
        seq->frames[1] = (KeyFrame){2.5, -2, {-0.2, -2, 0.5, 5}};
        seq->frames[2] = (KeyFrame){5, 270, {-0.2, -2, 0.5, 5}};
    }

    Anim *anim = xxmalloc(sizeof(Anim) * 1);
    anim->name = str("walk");
    anim->data = sequences;
    anim->length = 3;

    ctx.time = 0;
    ctx.anim = anim;



    skel = skeleton_cerate(vec2_zero);
    skeleton_loadfile(skel, "bones.skel");

}

static void render(Graph1Context *self)
{
    float x = anim_iterpolate(&ctx.anim->data[0], ctx.time);
    float y = anim_iterpolate(&ctx.anim->data[1], ctx.time);
    float r = anim_iterpolate(&ctx.anim->data[2], ctx.time);

    // draw_axisRot(vec3(0, x, y), 10, rot(0, 0, r));

    bone_set_wpos(skel, 4, vec2(x, y));

    skeleton_step(skel, gtime->delta);
    skeleton_render(skel);
}

static void render_after(Graph1Context *self)
{
    gui_begin();

    igSequencer("hello", 360, &ctx);


    gui_end();
}
static void destroy(Graph1Context *self)
{
    skeleton_destroy(skel);

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