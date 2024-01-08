#include "bone.h"

#include "mem/alloc.h"
#include "draw.h"

Skeleton2d *skeleton_cerate(Vec2 pos)
{
    Skeleton2d *self = xxmalloc(sizeof(Skeleton2d));
    self->bones = fastvec_Bone2d_init(8);
    fastvec_Bone2d_push(
        self->bones,
        (Bone2d){
            pos,
            vec2_up,
            vec2_zero,
            true,
        });
    return self;
}

void skeleton_add(Skeleton2d *self, Vec2 pos)
{
    Bone2d *prev = &self->bones->vector[self->bones->length - 1];
    prev->direction = vec2_sub(pos, prev->position);

    printf("%.2f, %.2f \n", pos.x, pos.y);
    fastvec_Bone2d_push(
        self->bones,
        (Bone2d){
            pos,
            vec2_up,
            vec2_zero,
            false,
        });
}
void skeleton_step(Skeleton2d *self, float dt)
{
    float ang = 0;
    float snapBackStrength = 0.5f;
    for (int i = 0; i < self->bones->length - 1; i++)
    {
        Bone2d *j1 = &self->bones->vector[i];
        float len = vec2_length(j1->direction);
        if (i == 0)
        {
            Vec2 d0 = j1->direction;
            Bone2d *j2 = &self->bones->vector[i + 1];
            Vec2 d1 = j2->direction;
            ang = -vec2_angle(d1, d0);
        }
        else
        {
            Bone2d *j0 = &self->bones->vector[i - 1];
            Bone2d *j2 = &self->bones->vector[i + 1];
            float angle = vec2_angle(vec2_sub(j1->position, j0->position), vec2_sub(j2->position, j1->position));
            Vec2 dir = rot2_rotate(rot2f(angle), vec2_right);
            j2->position = vec2_lerp(j2->position, vec2_mul_add(j1->position, len, dir), snapBackStrength);
            ang += angle;
        }
    }

    for (int iteration = 0; iteration < 3; iteration++)
    {
        for (int i = self->bones->length - 1; i >= 0; i--)
        {
            Bone2d *j1 = &self->bones->vector[i];
            float len = vec2_length(j1->direction);
            if (j1->attached)
            {
                j1->position = j1->target;
            }
            else
            {
                if (i < self->bones->length - 1)
                {
                    Bone2d *j2 = &self->bones->vector[i + 1];
                    Vec2 dir = vec2_norm(vec2_sub(j1->position, j2->position));
                    j1->position = vec2_lerp(j1->position, vec2_mul_add(j2->position, len, dir), snapBackStrength);
                }
            }
        }
        for (int i = 1; i < self->bones->length; i++)
        {
            Bone2d *j0 = &self->bones->vector[i - 1];
            Bone2d *j1 = &self->bones->vector[i];
            float len = vec2_length(j0->direction);
            Vec2 dir = vec2_norm(vec2_sub(j1->position, j0->position));
            j1->position = vec2_lerp(j1->position, vec2_mul_add(j0->position, len, dir), snapBackStrength);
        }
    }
}

void skeleton_free(Skeleton2d *self)
{

    fastvec_Bone2d_destroy(self->bones);
    xxfree(self, sizeof(Skeleton2d));
}