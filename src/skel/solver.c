#include "skel.h"
#include "skel_prv.h"
#include "bone.h"
#include "bone_prv.h"
#include "engine/draw.h"

void skeleton_add(Skel *self, Vec2 pos)
{
}

void solve_fabric(SkelPrv *skel, Constr *ctr)
{
    
    Bone *bones = skel->bones->vector;
    int n = ctr->n;
    if (n > 0)
    {
        Vec2 pole = bones[ctr->pole].world_position;
        Vec2 target = bones[ctr->target].world_position;
        for (int i = n - 1; i >= 0; i--)
        {
            Bone *it = &bones[ctr->bones[i]];
            Vec2 sub = vec2_sub(target, it->world_position0);
            it->world_rotation0 = clamp_axisf(-atan2df(sub.y, sub.x));
            it->world_position0 = vec2_mul_add(target, it->len, vec2_rotate(vec2_right, -it->world_rotation0 + 180));
            it->world_rotation = it->world_rotation0;
            target = it->world_position0;
        }
        target = bones[ctr->bones[0]].world_position;
        for (int i = 0; i < n; i++)
        {
            Bone *it = &bones[ctr->bones[i]];
            Vec2 d2 = vec2_mul_add(it->world_position0, it->len, vec2_rotate(vec2_right, -it->world_rotation0));
            Vec2 sub = vec2_sub(d2, target);
            it->world_rotation0 = clamp_axisf(-atan2df(sub.y, sub.x));
            it->world_position0 = target;
            it->world_rotation = it->world_rotation0;
            it->world = mat3_transform(it->world_position0, it->world_rotation0, it->local_scale);
            it->local = it->world;
            target = vec2_mul_add(target, it->len, vec2_rotate(vec2_right, -it->world_rotation0));
        }
    }
}
void solve_ccd(SkelPrv *skel, Constr *ctr)
{
    Bone *bones = skel->bones->vector;
    int n = ctr->n;
}
void skeleton_step(Skel *self, float dt)
{
    update_transforms(self);
    update_matrices(self, true);

    SkelPrv *skel = self->context;
    Constr *constrains = skel->constraints->vector;
    for (int c = 0; c < skel->constraints->length; c++)
    {
        Constr *ctr = &skel->constraints->vector[c];
        if (ctr->solver == CONSTR_SOLVER_FABRIC)
        {
            solve_fabric(skel, ctr);
        }
        else if (ctr->solver == CONSTR_SOLVER_CCD)
        {
            solve_ccd(skel, ctr);
        }
    }
}