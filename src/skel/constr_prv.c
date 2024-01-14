#pragma once

#include "constr_prv.h"

void solve_fabric(SkelPrv *skel, Constr *ctr)
{
    Bone *bones = skel->bones->vector;
    int n = ctr->n;
    if (n > 0)
    {
        Vec2 toTarget;
        Vec2 dir;
        Vec2 pole = bones[ctr->pole].world_position;
        Vec2 target = bones[ctr->target].world_position;
        Vec2 source = bones[ctr->bones[0]].world_position;
        Vec2 cen = vec2_center(target, source);
        Vec2 toPole = vec2_sub(cen, pole);
        toPole = vec2_mulfv(ctr->pole_stiffness * (vec2_length(toPole)/10), toPole);

        for (int i = n - 1; i >= 0; i--)
        {
            Bone *it = &bones[ctr->bones[i]];
            toTarget = vec2_sub(target, it->world_position0);
            toTarget = vec2_add(toTarget, toPole);
            
            it->world_rotation0 = clamp_axisf(-atan2df(toTarget.y, toTarget.x));
            dir = vec2_rotate(vec2_right, -it->world_rotation0 + 180);
            it->world_position0 = vec2_mul_add(target, it->len, dir);
            it->world_position = it->world_position0;
            it->world_rotation = it->world_rotation0;
            target = it->world_position0;
        }

        target = source;
        for (int i = 0; i < n; i++)
        {
            Bone *it = &bones[ctr->bones[i]];
            dir = vec2_rotate(vec2_right, -it->world_rotation0);
            Vec2 endPoint = vec2_mul_add(it->world_position0, it->len, dir);
            toTarget = vec2_sub(endPoint, target);
            it->world_rotation0 = clamp_axisf(-atan2df(toTarget.y, toTarget.x));
            it->world_position0 = target;
            it->world_position = it->world_position0;
            it->world_rotation = it->world_rotation0;
            dir = vec2_rotate(vec2_right, -it->world_rotation0);
            target = vec2_mul_add(target, it->len, dir);
            it->world = mat3_transform(it->world_position0, it->world_rotation0, it->local_scale);
            it->local = it->world;
        }
    }
}

void update_constraints(Skel *self)
{

    SkelPrv *skel = self->context;
    Constr *constrains = skel->constraints->vector;
    for (int c = 0; c < skel->constraints->length; c++)
    {
        Constr *ctr = &skel->constraints->vector[c];
        if (ctr->solver == CONSTR_SOLVER_FABRIC)
            solve_fabric(skel, ctr);
    }
}
