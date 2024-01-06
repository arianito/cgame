#ifndef cgame_VEC3_IFC_H
#define cgame_VEC3_IFC_H

typedef struct
{
    float x;
    float y;
    float z;
} Vec3;

static const Vec3 vec3_zero = {0, 0, 0};
static const Vec3 vec3_one = {1, 1, 1};
static const Vec3 vec3_forward = {1, 0, 0};
static const Vec3 vec3_backward = {-1, 0, 0};
static const Vec3 vec3_right = {0, 1, 0};
static const Vec3 vec3_left = {0, -1, 0};
static const Vec3 vec3_up = {0, 0, 1};
static const Vec3 vec3_down = {0, 0, -1};

#define vec3(a, b, c) ((Vec3){a, b, c})
#define vec3f(a) ((Vec3){a, a, a})

#endif