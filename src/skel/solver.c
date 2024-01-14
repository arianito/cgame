#include "skel.h"
#include "skel_prv.h"
#include "bone.h"
#include "bone_prv.h"
#include "engine/draw.h"

void skeleton_step(Skel *self, float dt)
{
    SkelPrv *skel = self->context;
    Bone *bones = skel->bones->vector;

    update_transforms(self);
    update_matrices(self, true);
    update_constraints(self);

}