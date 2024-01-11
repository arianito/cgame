#include "skel.h"
#include "skel_prv.h"
#include "bone.h"

void skeleton_add(Skel *self, Vec2 pos)
{
}

void skeleton_step(Skel *self, float dt)
{
    update_matrices(self);
    SkelPrv *skel = self->context;
#if 0
    for (int i = skel->bones->length - 1; i >= skel->dirty2; i--)
    {
#else
    for (int i = skel->dirty2; i < skel->bones->length; i++)
    {
#endif
        Bone *it = &skel->bones->vector[i];

        if (it->parent != -1)
        {
            Bone *pt = &skel->bones->vector[it->parent];
            if (!vec2_near_eq(it->world_position0, it->world_position))
            {
                Mat3 m2 = mat3_inv(pt->world);
                it->local_position = mat3_mulv2(m2, it->world_position0, 1);
                it->world_position = it->world_position0;
            }

            if (!near_eqf(it->world_rotation, it->world_rotation0))
            {
                Mat3 m2 = pt->world;
                it->local_rotation = it->world_rotation0 - clamp_axisf(atan2df(m2.m[1][0], m2.m[0][0]));
                it->world_rotation = it->world_rotation0;
            }
            if (!vec2_near_eq(it->world_scale0, it->world_scale))
            {
                Mat3 m2 = mat3_inv(pt->world);
                float det = m2.m[0][0] * m2.m[1][1] - m2.m[0][1] * m2.m[1][0];
                it->local_scale.x = sqrf(m2.m[0][0] * m2.m[0][0] + m2.m[1][0] * m2.m[1][0]);
                it->local_scale.y = it->world_scale0.y * (det / it->local_scale.x);
                it->local_scale.x *= it->world_scale0.x;
                it->world_scale = it->world_scale0;
            }
        }
        else
        {
            it->local_position = it->world_position0;
            it->local_rotation = it->world_rotation0;
            it->local_scale = it->world_scale0;
        }
        skel->dirty = minf(skel->dirty, i);
    }
    skel->dirty2 = skel->bones->length - 1;

    // SkelPrv* skel = self->context;
    // Fastvec_Bone *bones = skel->bones;
    // int n = bones->length;
    // Vec2 target = self->target;
    // for (int i = n - 1; i >= 0; i--)
    // {
    //     Bone *it = &bones->vector[i];
    //     Vec2 sub = vec2_sub(target, it->position);
    //     it->rotation = clamp_axisf(atan2df(sub.y, sub.x));
    //     it->position = vec2_mul_add(target, it->len, vec2_rotate(vec2_right, it->rotation + 180));
    //     target = it->position;
    // }
    // target = self->origin;
    // for (int i = 0; i < n; i++)
    // {
    //     Bone *it = &bones->vector[i];
    //     Vec2 d2 = vec2_mul_add(it->position, it->len, vec2_rotate(vec2_right, it->rotation));
    //     Vec2 sub = vec2_sub(d2, target);
    //     it->rotation = clamp_axisf(atan2df(sub.y, sub.x));
    //     it->position = target;
    //     target = vec2_mul_add(target, it->len, vec2_rotate(vec2_right, it->rotation));
    // }
}