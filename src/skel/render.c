#include "skel.h"

#include "engine/draw.h"
#include "skel_prv.h"
#include "engine/camera.h"
#include "engine/debug.h"
#include "math/quad.h"

#define BONE_THICKNESS 0.5

static Vec2 bone_pts[4] = {
    vec2(0.0, 0.0),
    vec2(BONE_THICKNESS, BONE_THICKNESS),
    vec2(1, 0),
    vec2(BONE_THICKNESS, -BONE_THICKNESS),
};
static Vec2 handle_pts[8] = {
    vec2(0.2, 0.0),
    vec2(1.0, 0.0),

    vec2(-0.2, 0.0),
    vec2(-0.5, 0.0),

    vec2(0.0, 0.2),
    vec2(0.0, 0.5),

    vec2(0.0, -0.2),
    vec2(0.0, -0.5),
};

static Bone *selected = NULL;

void skeleton_render(Skel *self)
{
    Ray r = camera_screenToWorld(input->position);
    Vec2 wp = vec2yz(vec3_intersect_plane(r.origin, vec3_mulf(r.direction, 1000), vec3_zero, vec3_forward));
    SkelPrv *skel = self->context;
    for (int i = 0; i < skel->bones->length; i++)
    {
        Bone *it = &skel->bones->vector[i];
        Mat3 tran = bone_world_matrix(self, i);

        float len = maxf(it->len, BONE_THICKNESS);
        float alpha = 0.5;
        Vec2 v0 = mat3_mulv2(tran, vec2_zero, 1);
        float rad = vec2_length(mat3_mulv2(tran, vec2_right, 1));

        switch (it->type)
        {
        case SKEL_TYP_ROOT:
            //
            if (input_mousedown(MOUSE_LEFT))
                if (ray_hit_sphere(r, sphere(vec3yz(v0), rad), NULL))
                    selected = it;
            if (it == selected)
                alpha = 1;
            //
            fill_circle_yz(vec3yzx(v0, -0.2), rad, color_alpha(color_red, alpha), 8, false);
            for (int j = 0; j < 4; j++)
            {
                Vec2 v1 = mat3_mulv2(tran, handle_pts[2 * j], 1);
                Vec2 v2 = mat3_mulv2(tran, handle_pts[2 * j + 1], 1);
                draw_line(vec3yz(v1), vec3yz(v2), color_white);
            }
            break;
        case SKEL_TYP_BONE:
            Vec2 pts[4];
            for (int j = 0; j < 4; j++)
            {
                Vec2 p = bone_pts[j];
                if (j == 2)
                    p.x *= len;
                pts[j] = mat3_mulv2(tran, p, 1);
            }
            draw_polygon_yz(pts, 4, color_blue);
            //
            if (input_mousedown(MOUSE_LEFT))
                if (ray_hit_quad(r, quad(vec3yz(pts[0]), vec3yz(pts[1]), vec3yz(pts[2]), vec3yz(pts[3])), NULL))
                    selected = it;
            if (it == selected)
                alpha = 1;
            //
            fill_polygon_yz(pts, 4, color_alpha(color_blue, alpha), false);
            debug_origin(vec2(0.5, 0.5));
            debug_scale(0.25);
            debug_rotation(rot(0, 0, -it->world_rotation));
            debug_string3df(vec3yzx(vec2_center(pts[0], pts[2]), 0.5), "%s", it->name);

            break;
        }
    }
    debug_reset();

    if (selected)
    {
        Bone *it = selected;
        Mat3 tran = bone_world_matrix(self, it->index);
        Vec2 v0 = mat3_mulv2(tran, vec2_zero, 1);

        if (input_keypress(KEY_R))
        {
            Vec2 dir = vec2_sub(wp, v0);
            bone_upd_world_rot(self, it->index, clamp_axisf(atan2df(dir.y, dir.x)));
        }

        debug_stringf(vec2(10, 40), "world_pos(%.2f, %.2f) world_rot(%.2f) world_scale(%.2f, %.2f) world_shear(%.2f, %.2f)",
                      it->world_position.x, it->world_position.y,
                      it->world_rotation,
                      it->world_scale.x, it->world_scale.y,
                      it->world_shear.x, it->world_shear.y);


        debug_stringf(vec2(10, 60), "local_pos(%.2f, %.2f) local_rot(%.2f) local_scale(%.2f, %.2f) local_shear(%.2f, %.2f)",
                      it->local_position.x, it->local_position.y,
                      it->local_rotation,
                      it->local_scale.x, it->local_scale.y,
                      it->local_shear.x, it->local_shear.y);
    }
}