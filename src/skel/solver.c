#include "skel.h"
#include "skel_prv.h"
#include "bone.h"
#include "bone_prv.h"

void skeleton_add(Skel *self, Vec2 pos)
{
}

void skeleton_step(Skel *self, float dt)
{
    update_matrices(self);
    update_transforms(self);

    // SkelPrv* skel = self->context;
    // Fastvec_Bone *bones = skel->bones;
    // int n = bones->length;
    // Vec2 target = self->target;
    // for (int i = n - 1; i >= 0; i--)
    // {
    //     Bone *it = &bones->vector[i];
    //     Vec2 sub = vec2_sub(target, it->position);
    //     it->rotation = clamp_axisf(atan2df(sub.y, sub.x));
    //     it->position = vec2_mul_add(target, it->len, vec2_rotate(vec2_right, it->rotation + 180));
    //     target = it->position;
    // }
    // target = self->origin;
    // for (int i = 0; i < n; i++)
    // {
    //     Bone *it = &bones->vector[i];
    //     Vec2 d2 = vec2_mul_add(it->position, it->len, vec2_rotate(vec2_right, it->rotation));
    //     Vec2 sub = vec2_sub(d2, target);
    //     it->rotation = clamp_axisf(atan2df(sub.y, sub.x));
    //     it->position = target;
    //     target = vec2_mul_add(target, it->len, vec2_rotate(vec2_right, it->rotation));
    // }
}