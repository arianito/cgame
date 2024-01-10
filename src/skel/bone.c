#include "skel.h"
#include "skel_prv.h"
#include "bone.h"

void bone_upd_world_rot(Skel *self, int bone, float rotation)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->world_rotation = rotation;
}

void bone_upd_local_rot(Skel *self, int bone, float rotation)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->local_rotation = rotation;
    bone_world_matrix(self, bone);
}

float bone_sum_rot(Skel *self, int bone)
{

    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    if (it->parent == -1)
        return it->local_rotation;

    return it->local_rotation + bone_sum_rot(self, it->parent);
}

void bone_upd_transform_bwd(Skel *self, int bone)
{
}

Mat3 bone_world_matrix(Skel *self, int bone)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->local = bone_local_matrix(self, bone);
    if (it->parent == -1)
    {
        it->world = it->local;
        return it->world;
    }
    else
    {
        it->world = mat3_mul(it->local, bone_world_matrix(self, it->parent));
        return it->world;
    }
}

Mat3 bone_local_matrix(Skel *self, int bone)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->local = mat3_transform(it->local_position, it->local_rotation, it->local_scale, it->local_shear);
    return it->local;
}
