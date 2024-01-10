#include "skel.h"
#include "skel_prv.h"
#include "bone.h"

void bone_upd_world_rot(Skel *self, int bone, float rotation)
{

    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->world_rotation = rotation;
}

void bone_upd_transform_bwd(Skel *self, int bone)
{
    // it->local = ;
}
Mat3 bone_world_matrix(Skel *self, int bone)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    if (it->parent == -1)
    {
        return bone_local_matrix(self, bone);
    }
    else
    {
        Bone *parent = &skel->bones->vector[it->parent];
    }
}
Mat3 bone_local_matrix(Skel *self, int bone)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    return mat3_transform(it->local_position, it->local_rotation, it->local_scale, it->local_shear);
}
