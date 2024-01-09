#include "bone.h"

#include "mem/alloc.h"
#include "draw.h"

Skeleton2d *skeleton_cerate(Vec2 pos)
{
    Skeleton2d *self = xxmalloc(sizeof(Skeleton2d));
    self->bones = fastvec_Bone2d_init(8);
    self->init = false;

    return self;
}

void skeleton_add(Skeleton2d *self, Vec2 pos)
{
    if (!self->init)
    {
        self->origin = pos;
        self->init = true;
        return;
    }

    Vec2 prev;
    if (self->bones->length == 0)
    {
        prev = self->origin;
    }
    else
    {
        Bone2d it = self->bones->vector[self->bones->length - 1];
        prev = vec2_mul_add(it.position, it.len, vec2_rotate(vec2_right, it.angle));
    }
    Bone2d it;
    it.position = prev;
    Vec2 sub = vec2_sub(pos, prev);
    it.angle = clamp_axisf(atan2df(sub.y, sub.x));
    it.len = vec2_distance(prev, pos);
    it.len0 = it.len;
    fastvec_Bone2d_push(self->bones, it);
}

void skeleton_step(Skeleton2d *self, float dt)
{
    Fastvec_Bone2d *bones = self->bones;
    int n = bones->length;

    Vec2 target = self->target;
    for(int i = n - 1; i >= 0; i--) {
        Bone2d *it = &bones->vector[i];
        Vec2 sub = vec2_sub(target, it->position);
        it->angle = clamp_axisf(atan2df(sub.y, sub.x));
        it->position =  vec2_mul_add(target, it->len, vec2_rotate(vec2_right, it->angle + 180));

        target = it->position;
    }
    target = self->origin;
    for(int i = 0; i < n; i++) {
        Bone2d *it = &bones->vector[i];

        Vec2 d2 = vec2_mul_add(it->position, it->len, vec2_rotate(vec2_right, it->angle));

        Vec2 sub = vec2_sub(d2, target);
        it->angle = clamp_axisf(atan2df(sub.y, sub.x));
        it->position = target;
        
        target = vec2_mul_add(target, it->len, vec2_rotate(vec2_right, it->angle));

    }
    // bones->vector[0] = vec2_zero;
    // for (int i = 1; i < n; i++)
    // {
    //     Vec2 *it = &bones->vector[i];
    //     Vec2 *prev = &bones->vector[i - 1];
    //     float len = lens->vector[i - 1];

    //     Vec2 dir = vec2_norm(vec2_sub(*it, *prev));
    //     *it = vec2_mul_add(*prev, len, dir);
    // }
}

void skeleton_free(Skeleton2d *self)
{

    fastvec_Bone2d_destroy(self->bones);
    xxfree(self, sizeof(Skeleton2d));
}