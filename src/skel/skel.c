#include "mem/mem.h"
#include "skel.h"
#include "skel_prv.h"

Skel *skeleton_cerate(Vec2 pos)
{
    Skel *self = xxmalloc(sizeof(Skel));
    self->context = xxmalloc(sizeof(SkelPrv));
    SkelPrv *skel = self->context;
    skel->bones = fastvec_Bone_init(8);
    return self;
}

void skeleton_free(Skel *self)
{
    SkelPrv *skel = self->context;
    fastvec_Bone_destroy(skel->bones);
    xxfree(skel, sizeof(SkelPrv));
    xxfree(self, sizeof(Skel));
}