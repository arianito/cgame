#ifndef cgame_RECT_H
#define cgame_RECT_H

typedef struct
{
    float a;
    float b;
    float c;
    float d;
} Rect;

#define rect(x, y, w, h) ((Rect){x, y, w, h})
#endif