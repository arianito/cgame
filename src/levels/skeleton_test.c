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

#include "math/mat3.h"

typedef struct
{
    Skel *skel;
} SkeletonTestbestContext;

static void create(SkeletonTestbestContext *self)
{
    atlas_load("platform", "textures/textures.png");
    mesh_load("bone", "models/bone.obj");
    gui_init(game->window, "fonts/roboto.ttf");

    Skel *skel = skeleton_cerate(vec2_zero);

    skeleton_loadfile(skel, "bones.skel");

    self->skel = skel;
}

static void render(SkeletonTestbestContext *self)
{
    Ray r = camera_screenToWorld(input->position);
    Vec3 wp = vec3_intersect_plane(r.origin, vec3_mulf(r.direction, 1000), vec3_zero, vec3_forward);
    Vec2 p = vec2yz(wp);

    Skel *skel = self->skel;
    skeleton_step(skel, gtime->delta);
    skeleton_render(skel);

    // Mat3 m = mat3_transform(vec2(10, 0), 0, vec2(1, 1), vec2(45, 0));
    // Vec2 ps[] = {
    //     vec2(-10, -10),
    //     vec2(-10, 10),
    //     vec2(10, 10),
    //     vec2(10, -10),
    // };
    // draw_polygon_yz(ps, 4, color_blue);
    // for (int i = 0; i < 4; i++)
    // {
    //     ps[i] = mat3_mulv2(m, ps[i], 1);
    // }
    // draw_polygon_yz(ps, 4, color_yellow);
}

static void render_after(SkeletonTestbestContext *self)
{
    // gui_begin();

    // ImGuiID id = igDockSpace(igGetID_Ptr("Dockspace"), (ImVec2){300, 500}, ImGuiDockNode_IsRootNode, NULL);
    // float toolbar = 360;

    // igBegin("toolbar", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

    // igText("temp world");
    // igButton("hello", (ImVec2){100, 28});

    // igSetWindowPos_Vec2((ImVec2){game->size.x - igGetWindowWidth(), game->size.y - igGetWindowHeight()}, 0);
    // igEnd();

    // gui_end();
}

static void destroy(SkeletonTestbestContext *self)
{
    skeleton_free(self->skel);

    atlas_clear();
    sprite_clear();
    mesh_clear();
    gui_destroy();
}

Level make_skeleton_testbed()
{
    return (Level){
        context : xxarena(sizeof(SkeletonTestbestContext)),
        create : &create,
        render : &render,
        render_after : &render_after,
        destroy : &destroy,
    };
}