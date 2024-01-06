#ifndef cgame_CAMERA_H
#define cgame_CAMERA_H

#include "math/vec3.h"
#include "math/rot.h"
#include "math/mat4.h"
#include "math/ray.h"

#include "game.h"
#include "input.h"

#include "mem/alloc.h"

typedef enum
{
    VIEW_INITIAL = 0,
    VIEW_ORTHOGRAPHIC = 1 << 0,
    VIEW_BACK = 1 << 1,
    VIEW_FRONT = 1 << 2,
    VIEW_LEFT = 1 << 3,
    VIEW_RIGHT = 1 << 4,
    VIEW_BOTTOM = 1 << 5,
    VIEW_TOP = 1 << 6

} OrthoMode;

typedef struct
{
    float far_plane;
    Vec3 position;
    Rot rotation;
    float zoom;
    Mat4 view;
    Mat4 projection;
    float fov;
    Mat4 view_projection;
    int ortho;
} Camera;

extern Camera *camera;

void camera_update();
void camera_init();
Vec2 camera_worldToScreen(Vec3 p);
Ray camera_screenToWorld(Vec2 s);

#endif