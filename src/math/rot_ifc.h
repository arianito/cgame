#ifndef cgame_ROT_IFC_H
#define cgame_ROT_IFC_H


typedef struct
{
    float pitch;
    float yaw;
    float roll;
} Rot;


#define rot(pitch, yaw, roll) ((Rot){pitch, yaw, roll})

static const Rot rot_zero = {0, 0, 0};

#endif