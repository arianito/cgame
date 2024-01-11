#include "skel.h"
#include "skel_prv.h"
#include "bone.h"

void make_dirty_fwd(Skel *self, int bone)
{
    SkelPrv *skel = self->context;
    for (int i = 0; i < skel->bones->length; i++)
        skel->bones->vector[i].dirty = 1;
}

void bone_upd_transform(Skel *self, int bone)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    if (it->parent != -1)
    {
        Bone *pt = &skel->bones->vector[it->parent];
        Mat3 m2 = mat3_inv(pt->world);

        float dx = it->world_position.x - pt->world_position.x;
        float dy = it->world_position.y - pt->world_position.y;
        it->local_position.x = dx * m2.m[0][0] + dy * m2.m[1][0];
        it->local_position.y = dx * m2.m[0][1] + dy * m2.m[1][1];

        // m2 = mat3_mul(m2, it->local);
        // clamp_axisf(atan2df(m2.m[1][0], m2.m[0][0]));
        it->local_rotation = it->world_rotation - pt->world_rotation;

        float det = m2.m[0][0] * m2.m[1][1] - m2.m[0][1] * m2.m[1][0];
        it->local_scale.x = sqrf(m2.m[0][0] * m2.m[0][0] + m2.m[1][0] * m2.m[1][0]);
        it->local_scale.y = det / it->local_scale.x;
    }
    else
    {
        it->local_position = it->world_position;
        it->local_rotation = it->world_rotation;
        it->local_scale = it->world_scale;
    }
}
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
    make_dirty_fwd(self, bone);
}

void bone_upd_local_rot(Skel *self, int bone, float rotation)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    it->local_rotation = clamp_axisf(rotation);
    make_dirty_fwd(self, bone);
}

void update_matrices(Skel *self, int bone)
{
    SkelPrv *skel = self->context;
    Bone *it = &skel->bones->vector[bone];
    if (!it->dirty)
        return;

    it->local = mat3_transform(it->local_position, it->local_rotation, it->local_scale);

    if (it->parent == -1)
    {
        it->world = it->local;
        it->dirty = false;
        return;
    }

    update_matrices(self, it->parent);
    Bone *pt = &skel->bones->vector[it->parent];
    it->world = mat3_mul(it->local, pt->world);
    it->dirty = 0;
}
