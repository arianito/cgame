#ifndef cgame_MAT4_IFC_H
#define cgame_MAT4_IFC_H


typedef struct
{
    float m[4][4];
} Mat4;

#define mat4(a) ((Mat4){{{a, 0, 0, 0}, {0, a, 0, 0}, {0, 0, a, 0}, {0, 0, 0, a}}})
static const Mat4 mat4_identity = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};

#endif