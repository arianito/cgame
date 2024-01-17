#include "anim.h"

static Vec2 cubic_bezier(Vec2 P0, Vec2 P1, Vec2 P2, Vec2 P3, float t)
{
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
    Vec2 qs[4];
    anim_control_points(&seq->frames[i0], &seq->frames[i1], qs);
    return cubic_bezier(qs[0], qs[1], qs[2], qs[3], t).y;
}

void anim_control_points(KeyFrame *pkf, KeyFrame *kf, Vec2 qs[4])
{
    float jmpX = kf->t - pkf->t;
    float jmpY = kf->value - pkf->value;

    qs[0].x = pkf->t;
    qs[0].y = pkf->value;

    qs[1].x = pkf->t + (1 - pkf->cubic[2]) * jmpX;
    qs[1].y = pkf->value + (1 - pkf->cubic[3]) * jmpY;

    qs[2].x = kf->t - kf->cubic[0] * jmpX;
    qs[2].y = kf->value - kf->cubic[1] * jmpY;

    qs[3].x = kf->t;
    qs[3].y = kf->value;
}
