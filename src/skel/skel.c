#include "mem/mem.h"
#include "mem/defs.h"
#include "skel.h"
#include "skel_prv.h"

Skel *skeleton_cerate(Vec2 pos)
{
    Skel *self = xxmalloc(sizeof(Skel));
    self->context = xxmalloc(sizeof(SkelPrv));
    SkelPrv *skel = self->context;
    skel->dirty = 1;
    skel->dirty2 = 0;
    skel->bones = fastvec_Bone_init(8);
    skel->buffer = make_arena(4 * KILOBYTES);
    skel->map = fastmap_StrInt_init();
    return self;
}

void skeleton_free(Skel *self)
{
    SkelPrv *skel = self->context;
    fastmap_StrInt_destroy(skel->map);
    arena_destroy(skel->buffer);
    fastvec_Bone_destroy(skel->bones);
    xxfree(skel, sizeof(SkelPrv));
    xxfree(self, sizeof(Skel));
}