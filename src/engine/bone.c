#include "bone.h"

#include "mem/alloc.h"
#include "draw.h"

Skeleton2d *skeleton_cerate(Vec2 pos)
{
    Skeleton2d *self = xxmalloc(sizeof(Skeleton2d));
    self->bones = fastvec_Bone_init(8);
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
        Bone it = self->bones->vector[self->bones->length - 1];
        prev = vec2_mul_add(it.position, it.len, vec2_rotate(vec2_right, it.angle));
    }
    Bone it;
    it.position = prev;
    Vec2 sub = vec2_sub(pos, prev);
    it.angle = clamp_axisf(atan2df(sub.y, sub.x));
    it.len = vec2_distance(prev, pos);
    fastvec_Bone_push(self->bones, it);
}

void skeleton_step(Skeleton2d *self, float dt)
{
    Fastvec_Bone *bones = self->bones;
    int n = bones->length;
    Vec2 target = self->target;
    for (int i = n - 1; i >= 0; i--)
    {
        Bone *it = &bones->vector[i];
        Vec2 sub = vec2_sub(target, it->position);
        it->angle = clamp_axisf(atan2df(sub.y, sub.x));
        it->position = vec2_mul_add(target, it->len, vec2_rotate(vec2_right, it->angle + 180));
        target = it->position;
    }
    target = self->origin;
    for (int i = 0; i < n; i++)
    {
        Bone *it = &bones->vector[i];
        Vec2 d2 = vec2_mul_add(it->position, it->len, vec2_rotate(vec2_right, it->angle));
        Vec2 sub = vec2_sub(d2, target);
        it->angle = clamp_axisf(atan2df(sub.y, sub.x));
        it->position = target;
        target = vec2_mul_add(target, it->len, vec2_rotate(vec2_right, it->angle));
    }
}
void skeleton_render(Skeleton2d *self)
{

    for (int i = 0; i < self->bones->length; i++)
    {
        Bone it = self->bones->vector[i];
        fill_circle_yz(vec3yz(it.position), 2, color_red, 6, false);
        draw_capsule_yz(it.position, vec2_mul_add(it.position, it.len, vec2_rotate(vec2_right, it.angle)), 1, color_blue, 6);
    }
}
void skeleton_free(Skeleton2d *self)
{

    fastvec_Bone_destroy(self->bones);
    xxfree(self, sizeof(Skeleton2d));
}