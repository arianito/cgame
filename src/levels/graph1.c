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
static Skel *skel;

static void create(Graph1Context *self)
{
    gui_init(NULL);

    camera->offset = 0.25f;
    camera_update();

    Anim *anim = xxmalloc(sizeof(Anim) * 1);
    anim->name = str("walk");
    anim->sequences = fastvec_AnimSeq_init(2);
    {
        AnimSequence seq = {0};
        seq.type = ANIM_PROP_Y;
        seq.frames = fastvec_KeyFrame_init(2);
        fastvec_KeyFrame_push(seq.frames, (KeyFrame){0, 10, {0}});
        fastvec_KeyFrame_push(seq.frames, (KeyFrame){1, 20, {0}});
        fastvec_KeyFrame_push(seq.frames, (KeyFrame){2, -50, {0}});
        fastvec_KeyFrame_push(seq.frames, (KeyFrame){3, 10, {0}});

        fastvec_AnimSeq_push(anim->sequences, seq);
    }

    ctx.time = 0;
    ctx.anim = anim;

    skel = skeleton_cerate(vec2_zero);
    skeleton_loadfile(skel, "bones.skel");
}

static void render(Graph1Context *self)
{
    float y = anim_iterpolate(&ctx.anim->sequences->vector[0], ctx.time);

    // draw_axisRot(vec3(0, x, y), 10, rot(0, 0, r));

    bone_set_wpos(skel, 4, vec2(0, y));

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