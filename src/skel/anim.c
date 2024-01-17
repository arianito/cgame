#include "anim.h"


static Vec2 cubic_bezier(Vec2 P0, Vec2 P1, Vec2 P2, Vec2 P3, float t) {
    Vec2 result;
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    result.x = uuu * P0.x + 3 * uu * t * P1.x + 3 * u * tt * P2.x + ttt * P3.x;
    result.y = uuu * P0.y + 3 * uu * t * P1.y + 3 * u * tt * P2.y + ttt * P3.y;

    return result;
}


float anim_iterpolate(AnimSequence *seq, float time)
{
    if (seq->length < 2)
        return 0;
    if (time < seq->frames[seq->i0].t || time >= seq->frames[seq->i0 + 1].t)
    {
        int low = 0;
        int high = seq->length - 1;
        while (low < high)
        {
            int mid = (low + high) / 2;
            if (time > seq->frames[mid].t)
                low = mid + 1;
            else
                high = mid;
        }
        seq->i0 = maxf(low - 1, 0);
    }
    int i0 = seq->i0;
    int i1 = seq->i0 + 1;

    float t = (time - seq->frames[i0].t) / (seq->frames[i1].t - seq->frames[i0].t);
    return lerp01f(seq->frames[i0].value, seq->frames[i1].value, t);
}

void anim_control_points(KeyFrame *pkf, KeyFrame *kf, Vec2 *q1, Vec2 *q2, Vec2 *q3, Vec2 *q4)
{
    float jmpX = kf->t - pkf->t;
    float jmpY = kf->value - pkf->value;

    q1->x = pkf->t;
    q1->y = pkf->value;

    q2->x = pkf->t + (1 - pkf->cubic[2]) * jmpX;
    q2->y = pkf->value + (1 - pkf->cubic[3]) * jmpY;

    q3->x = kf->t - kf->cubic[0] * jmpX;
    q3->y = kf->value - kf->cubic[1] * jmpY;

    q4->x = kf->t;
    q4->y = kf->value;
}
