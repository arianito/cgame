#include "skel.h"
#include "skel_prv.h"
#include "bone.h"

void bone_upd_world_rot(Skel *self, int bone, float rotation)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->world_rotation = clamp_axisf(rotation);
    if (it->parent != -1)
    {
        Bone *pt = &skel->bones->vector[it->parent];
        it->local_rotation = it->world_rotation - pt->world_rotation;
    }
    else
    {
        it->local_rotation = it->world_rotation;
    }

    skel->dirty = minf(skel->dirty, bone);
}
void bone_upd_world_pos(Skel *self, int bone, Vec2 pos)
{

    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->world_position = pos;
    if (it->parent != -1)
    {
        Bone *pt = &skel->bones->vector[it->parent];
        it->local_position = vec2_sub(it->world_position, pt->world_position);
    }
    else
    {
        it->local_position = it->world_position;
    }
    skel->dirty = minf(skel->dirty, bone);
}

void bone_upd_local_rot(Skel *self, int bone, float rotation)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->local_rotation = clamp_axisf(rotation);
}

void update_matrices(Skel *self)
{
    SkelPrv *skel = self->context;
    int from_index = skel->dirty;
    for (int i = from_index; i < skel->bones->length; i++)
    {
        skel->dirty = i;

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
