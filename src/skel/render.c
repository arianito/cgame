#include "skel.h"

#include "engine/draw.h"
#include "skel_prv.h"


void skeleton_render(Skel *self)
{
    SkelPrv* skel = self->context;

    for (int i = 0; i < skel->bones->length; i++)
    {
        Bone it = skel->bones->vector[i];
        draw_point(vec3yz(it.position), 0.5, color_red);
        draw_line(vec3yz(it.position), vec3yz(vec2_mul_add(it.position, it.len, vec2_rotate(vec2_right, it.rotation))), color_blue);
    }
}