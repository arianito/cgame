#ifndef cgame_PLANE_H
#define cgame_PLANE_H

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} Plane;

#define plane(x, y, z, w) ((Plane){x, y, z, w})

#endif