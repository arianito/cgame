#include "skel.h"
#include "skel_prv.h"
#include "bone.h"

void bone_set_wpos(Skel *self, int bone, Vec2 pos)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->world_position0 = pos;
    skel->dirty_world = minf(skel->dirty_world, bone);
}
void bone_set_wrot(Skel *self, int bone, float rotation)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->world_rotation0 = clamp_axisf(rotation);
    skel->dirty_world = minf(skel->dirty_world, bone);
}

void bone_set_wscale(Skel *self, int bone, Vec2 scale)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->world_scale0 = scale;
    skel->dirty_world = minf(skel->dirty_world, bone);
}

void bone_set_lpos(Skel *self, int bone, Vec2 pos)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->local_position = pos;
    skel->dirty_local = minf(skel->dirty_local, bone);
}

void bone_set_lrot(Skel *self, int bone, float rotation)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->local_rotation = clamp_axisf(rotation);
    skel->dirty_local = minf(skel->dirty_local, bone);
}

void bone_set_lscale(Skel *self, int bone, Vec2 scale)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->local_scale = scale;
    skel->dirty_local = minf(skel->dirty_local, bone);
}