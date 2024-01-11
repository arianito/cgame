#include "skel.h"
#include "skel_prv.h"
#include "bone.h"

void bone_upd_world_pos(Skel *self, int bone, Vec2 pos)
{

    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->world_position0 = pos;
    skel->dirty2 = minf(skel->dirty, bone);
}
void bone_upd_world_rot(Skel *self, int bone, float rotation)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->world_rotation0 = clamp_axisf(rotation);
    skel->dirty2 = minf(skel->dirty, bone);
}

void bone_upd_world_scale(Skel *self, int bone, Vec2 scale)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->world_scale0 = scale;
    skel->dirty2 = minf(skel->dirty, bone);
}


void bone_upd_local_rot(Skel *self, int bone, float rotation)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->local_rotation = clamp_axisf(rotation);
    skel->dirty = minf(skel->dirty, bone);
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
