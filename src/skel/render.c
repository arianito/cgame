#include "skel.h"

#include "engine/draw.h"
#include "skel_prv.h"


void skeleton_render(Skel *self)
{
    SkelPrv* skel = self->context;

    for (int i = 0; i < skel->bones->length; i++)
    {
        Bone it = skel->bones->vector[i];
        fill_circle_yz(vec3yz(it.position), 2, color_red, 6, false);
        draw_capsule_yz(it.position, vec2_mul_add(it.position, it.len, vec2_rotate(vec2_right, it.rotation)), 1, color_blue, 6);
    }
}