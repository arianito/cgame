#include "vec2.h"

#include <stdbool.h>
#include <stdint.h>
#include <float.h>
#include <math.h>

#include "defs.h"
#include "scalar.h"

bool vec2_valid(Vec2 a)
{
    if (isnan(a.x) || isnan(a.y))
        return false;
    if (isinf(a.x) || isinf(a.y))
        return false;
    return true;
}
