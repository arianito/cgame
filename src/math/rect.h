#ifndef cgame_RECT_H
#define cgame_RECT_H

#include "vec2.h"

typedef struct
{
    float a;
    float b;
    float c;
    float d;
} Rect;

#define rect(x, y, w, h) ((Rect){x, y, w, h})

static inline Rect rectv(Vec2 pos, Vec2 size) { return rect(pos.x, pos.y, size.x, size.y); }

#endif