#include "anim.h"

float animate(KeyFrame *keys, int n, int *id, float time)
{
    if (n < 2)
        return 0;
    if (time < keys[*id].t || time >= keys[*id + 1].t)
    {
        int low = 0;
        int high = n - 1;
        while (low < high)
        {
            int mid = (low + high) / 2;
            if (time > keys[mid].t)
                low = mid + 1;
            else
                high = mid;
        }
        *id = maxf(low - 1, 0);
    }
    int i0 = *id;
    int i1 = *id + 1;

    float t = (time - keys[i0].t) / (keys[i1].t - keys[i0].t);
    return lerp01f(keys[i0].value, keys[i1].value, t);
}