#ifndef cgame_MATH_DEFS_H
#define cgame_MATH_DEFS_H

#define USE_AVX_256
// #define USE_AVX_128

#define RAD2DEG (57.295779513082321f)
#define DEG2RAD (0.017453292519943f)
#define PI (3.141592653589793f)
#define HALF_PI (1.570796326794896f)
#define EPSILON (0.00001f)
#define EPSILON2 (0.0000000001f)
#define MAX_FLOAT (3.40282e+38f)
#define MIN_FLOAT (1.17549e-38f)

typedef enum
{
    UNIT_AXIS_X,
    UNIT_AXIS_Y,
    UNIT_AXIS_Z,
} UnitAxisEnum;

#endif