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
#include "engine/bone.h"

#include "engine/file.h"
typedef struct
{
    Skeleton2d *skel;
} SkeletonTestbestContext;

static void create(SkeletonTestbestContext *self)
{
    atlas_load("platform", "textures/textures.png");
    mesh_load("bone", "models/bone.obj");
    gui_init(game->window, "fonts/roboto.ttf");

    Skeleton2d *skel = skeleton_cerate(vec2_zero);

    Vec2 poses[4] = {
        vec2(0, 0),
        vec2(10, 20),
        vec2(-10, 40),
        vec2(0, 60),
    };
    for (int i = 0; i < 4; i++)
    {
        skeleton_add(skel, poses[i]);
    }

    skel->target = vec2(0, 70);

    self->skel = skel;
    // {
    //     SpriteId id = sprite_create("bone", "platform");
    //     Sprite *sp = sprite_get(id);
    //     sp->scale = vec3(10, 10, 10);
    //     sp->position = vec3(0, 0, 0);
    //     sprite_crop_pixelart_id(id, 0x021D1010);
    // }
}
static void render(SkeletonTestbestContext *self)
{
    Ray r = camera_screenToWorld(input->position);
    Vec3 wp = vec3_intersect_plane(r.origin, vec3_mulf(r.direction, 1000), vec3_zero, vec3_forward);
    Vec2 p = vec2yz(wp);

    Skeleton2d *skel = self->skel;

    if (input_mousepress(MOUSE_LEFT))
    {
        skel->target = p;
    }

    skeleton_step(skel, 0.2);

    for (int i = 0; i < skel->bones->length; i++)
    {
        Bone2d it = skel->bones->vector[i];
        fill_circle_yz(vec3yz(it.position), 2, color_red, 6, false);
        draw_capsule_yz(it.position, vec2_mul_add(it.position, it.len, vec2_rotate(vec2_right, it.angle)), 1, color_blue, 6);
    }

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