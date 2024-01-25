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
    if (seq->frames->length < 2)
        return 0;
    if (time < seq->frames->vector[seq->i0].t || time >= seq->frames->vector[seq->i0 + 1].t)
    {
        int low = 0;
        int high = seq->frames->length - 1;
        while (low < high)
        {
            int mid = (low + high) / 2;
            if (time > seq->frames->vector[mid].t)
                low = mid + 1;
            else
                high = mid;
        }
        seq->i0 = maxf(low - 1, 0);
    }
    int i0 = seq->i0;
    int i1 = seq->i0 + 1;

    float t = (time - seq->frames->vector[i0].t) / (seq->frames->vector[i1].t - seq->frames->vector[i0].t);
    Vec2 qs[4];
    anim_control_points(&seq->frames->vector[i0], &seq->frames->vector[i1], qs);
    return cubic_bezier(qs[0], qs[1], qs[2], qs[3], t).y;
}

void anim_control_points(KeyFrame *pkf, KeyFrame *kf, Vec2 qs[4])
{
    qs[0] = (Vec2){pkf->t, pkf->value};
    qs[1] = (Vec2){pkf->t + pkf->cubic[2], pkf->value + pkf->cubic[3]};
    
    qs[2] = (Vec2){kf->t + kf->cubic[0], kf->value + kf->cubic[1]};
    qs[3] = (Vec2){kf->t, kf->value};
}

KeyFrame *anim_find(AnimSequence *seq, float time, float epsilon)
{
    int low = 0;
    int high = seq->frames->length - 1;
    while (low <= high)
    {
        int mid = low + (high - low) / 2;
        float d = time - seq->frames->vector[mid].t;
        if (d > epsilon)
            low = mid + 1;
        else if (d < -epsilon)
            high = mid - 1;
        else
            return &seq->frames->vector[mid];
    }

    return NULL;
}

KeyFrame *anim_find_value(AnimSequence *seq, float value, float epsilon) {
    for(int i = 0; i < seq->frames->length; i++)
    {
        float d = value - seq->frames->vector[i].value;
        if (d <= epsilon && d >= -epsilon)
            return &seq->frames->vector[i];
    }
    return NULL;
}