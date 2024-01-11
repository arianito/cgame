
#include "skel_prv.h"
#include "bone_prv.h"
#include "bone.h"

void bone_upd_transform(Skel *self, int bone)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    if (it->parent != -1)
    {
        Bone *pt = &skel->bones->vector[it->parent];
        Mat3 m2 = mat3_inv(pt->world);
        it->local_position = mat3_mulv2(m2, it->world_position0, 1);
        it->local_rotation = clamp_axisf(atan2df(m2.m[1][0], m2.m[0][0]));

        float det = m2.m[0][0] * m2.m[1][1] - m2.m[0][1] * m2.m[1][0];
        it->local_scale.x = sqrf(m2.m[0][0] * m2.m[0][0] + m2.m[1][0] * m2.m[1][0]);
        it->local_scale.y = det / it->local_scale.x;

        it->local = mat3_transform(it->local_position, it->local_rotation, it->local_scale);
        it->world = mat3_mul(it->local, pt->world);
    }
    else
    {
        it->local_position = it->world_position0;
        it->local_rotation = it->world_rotation0;
        it->local_scale = it->world_scale0;
        it->world = mat3_transform(it->local_position, it->local_rotation, it->local_scale);
        it->local = it->world;
    }
    it->world_position = it->world_position0;
    it->world_rotation = it->world_rotation0;
    it->world_scale = it->world_scale0;
}

void update_matrices(Skel *self)
{
    SkelPrv *skel = self->context;
    int from_index = skel->dirty_local;
    for (int i = from_index; i < skel->bones->length; i++)
    {
        skel->dirty_local = i;

        Bone *it = &skel->bones->vector[i];
        it->local = mat3_transform(it->local_position, it->local_rotation, it->local_scale);

        if (it->parent == -1)
        {
            it->world = it->local;
            continue;
        }

        Bone *pt = &skel->bones->vector[it->parent];
        it->world = mat3_mul(it->local, pt->world);
    }
}

void update_transforms(Skel *self)
{
    SkelPrv *skel = self->context;
    int from_index = skel->dirty_world;
    for (int i = from_index; i < skel->bones->length; i++)
    {
        skel->dirty_world = i;
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
            // if (!vec2_near_eq(it->world_scale0, it->world_scale))
            // {
            Mat3 m2 = mat3_inv(pt->world);
            float det = m2.m[0][0] * m2.m[1][1] - m2.m[0][1] * m2.m[1][0];
            it->local_scale.x = sqrf(m2.m[0][0] * m2.m[0][0] + m2.m[1][0] * m2.m[1][0]);
            it->local_scale.y = it->world_scale0.y * (det / it->local_scale.x);
            it->local_scale.x *= it->world_scale0.x;
            it->world_scale = it->world_scale0;
            // }
            if (!near_eqf(it->world_rotation, it->world_rotation0))
            {
                Mat3 m2 = pt->world;
                it->local_rotation = it->world_rotation0 - clamp_axisf(atan2df(m2.m[1][0], m2.m[0][0]));
                it->world_rotation = it->world_rotation0;
            }
        }
        else
        {
            it->world_position = it->world_position0;
            it->world_rotation = it->world_rotation0;
            it->world_scale = it->world_scale0;
            //
            it->local_position = it->world_position0;
            it->local_rotation = it->world_rotation0;
            it->local_scale = it->world_scale0;
        }
        skel->dirty_local = minf(skel->dirty_local, i);
    }
}