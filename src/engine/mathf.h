#ifndef cgame_MATHF_H
#define cgame_MATHF_H

#include <immintrin.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#define USE_AVX_256
// #define USE_AVX_128

typedef struct
{
    float x;
    float y;
} Vec2;

typedef struct
{
    int x;
    int y;
} Vec2i;

typedef struct
{
    float x;
    float y;
    float z;
} Vec3;

typedef struct
{
    int x;
    int y;
    int z;
} Vec3i;

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} Vec4;

typedef struct
{
    Vec3 position;
    Vec2 coord;
} VertexData;

typedef struct
{
    Vec3 origin;
    Vec3 direction;
} Ray;

typedef struct
{
    float pitch;
    float yaw;
    float roll;
} Rot;

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} Quat;

typedef struct
{
    float a;
    float b;
    float c;
    float d;
} Rect;

typedef struct
{
    Vec3 position;
    Rot rotation;
    Vec3 scale;
} Transform;

typedef struct
{
    Vec3 position;
    float radius;
} Sphere;

typedef struct
{
    Vec3 a;
    Vec3 b;
    Vec3 c;
} Triangle;

typedef struct
{

    Vec3 a;
    Vec3 b;
    Vec3 c;
    Vec3 d;
} Quad;
typedef struct
{
    Vec3 a;
    Vec3 b;
} Edge;

typedef struct
{
    Vec3 a;
    Vec3 b;
    Vec3 c;
    Vec3 d;
} Tetrahedron;

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} Plane;

typedef struct
{
    float r;
    float g;
    float b;
    float a;
} Color;

typedef struct
{
    float h;
    float s;
    float l;
    float a;
} ColorHSL;

typedef struct
{
    Vec3 position;
    Color color;
    float size;
} Vertex;

typedef struct
{
    Vec3 min;
    Vec3 max;
} BBox;

typedef struct
{
    float m[4][4];
} Mat4;

typedef enum
{
    UNIT_AXIS_X,
    UNIT_AXIS_Y,
    UNIT_AXIS_Z,
} UnitAxisEnum;

#define RAD2DEG (57.295779513082321f)
#define DEG2RAD (0.017453292519943f)
#define PI (3.141592653589793f)
#define HALF_PI (1.570796326794896f)
#define EPSILON (0.00001f)
#define EPSILON2 (0.0000000001f)
#define MIN (1.17549e-38f)
#define MAX (3.40282e+38f)

static const Vec2 vec2_zero = {0, 0};
static const Vec2 vec2_one = {1, 1};
static const Vec2 vec2_up = {0, 1};
static const Vec2 vec2_down = {0, -1};
static const Vec2 vec2_left = {-1, 0};
static const Vec2 vec2_right = {1, 0};

static const Vec3 vec3_zero = {0, 0, 0};
static const Vec3 vec3_one = {1, 1, 1};
static const Vec3 vec3_forward = {1, 0, 0};
static const Vec3 vec3_backward = {-1, 0, 0};
static const Vec3 vec3_right = {0, 1, 0};
static const Vec3 vec3_left = {0, -1, 0};
static const Vec3 vec3_up = {0, 0, 1};
static const Vec3 vec3_down = {0, 0, -1};

static const Vec4 vec4_zero = {0, 0, 0, 0};
static const Vec4 vec4_one = {1, 1, 1, 1};

static const Color color_black = {0, 0, 0, 1};
static const Color color_white = {1, 1, 1, 1};
static const Color color_red = {0.737f, 0.278f, 0.286f, 1};
static const Color color_green = {0.655f, 0.788f, 0.341f, 1};
static const Color color_blue = {0.129f, 0.619f, 0.737f, 1};
static const Color color_yellow = {1.0f, 0.718f, 0.012f, 1};
static const Color color_orange = {0.984f, 0.521f, 0, 1};
static const Color color_gray = {0.5f, 0.5f, 0.5f, 1};

static const Quad quad_zero = {0, 0, 0, 0};

static const Quat quat_identity = {0, 0, 0, 1};
static const Rot rot_zero = {0, 0, 0};
static const Mat4 mat4_identity = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};

static const Transform transform_identity = {{0, 0, 0},
                                             {0, 0, 0},
                                             {1, 1, 1}};

static const BBox bbox_empty = {{MAX, MAX, MAX},
                                {MIN, MIN, MIN}};

static const BBox bbox_zero = {{0, 0, 0},
                               {0, 0, 0}};

// math
static inline void seedf(unsigned int seed)
{
    srand(seed);
}

#define IMAX_BITS(m) ((m) / ((m) % 255 + 1) / 255 % 255 * 8 + 7 - 86 / ((m) % 255 + 12))
#define RAND_MAX_WIDTH IMAX_BITS(RAND_MAX)

static inline float randf()
{
    unsigned int r = 0;
    for (int i = 0; i < 32; i += RAND_MAX_WIDTH)
    {
        r <<= RAND_MAX_WIDTH;
        r ^= (unsigned int)rand(); // NOLINT(*-msc50-cpp)
    }
    return (float)r / (float)0xffffffff;
}

inline static unsigned char log2i(unsigned int value)
{
    static const unsigned char table[64] =
        {
            63, 0, 58, 1, 59, 47, 53, 2,
            60, 39, 48, 27, 54, 33, 42, 3,
            61, 51, 37, 40, 49, 18, 28, 20,
            55, 30, 34, 11, 43, 14, 22, 4,
            62, 57, 46, 52, 38, 26, 32, 41,
            50, 36, 17, 19, 29, 10, 13, 21,
            56, 45, 25, 31, 35, 16, 9, 12,
            44, 24, 15, 8, 23, 7, 6, 5};
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return table[((value - (value >> 1)) * 0x07EDD5E59A4E28C2) >> 58];
}

static inline float sind(float d) { return sinf(d * DEG2RAD); }

static inline float cosd(float d) { return cosf(d * DEG2RAD); }

static inline float tand(float d) { return tanf(d * DEG2RAD); }

static inline float tanhd(float d) { return tanhf(d * DEG2RAD); }

static inline float sinhd(float d) { return sinhf(d * DEG2RAD); }

static inline float coshd(float d) { return coshf(d * DEG2RAD); }

static inline float ramp(float a) { return a < 0 ? 0 : a; }

static inline float imaxi(int a, int b) { return a > b ? a : b; }

static inline float imini(int a, int b) { return a < b ? a : b; }

static inline int floori(float a) { return (int)floorf(a); }

static inline float asind(float a) { return asinf(a) * RAD2DEG; }

static inline float square(float a) { return a * a; }

static inline float acosd(float a) { return acosf(a) * RAD2DEG; }

static inline float atand(float a) { return atanf(a) * RAD2DEG; }

static inline float atan2d(float a, float b) { return atan2f(a, b) * RAD2DEG; }

static inline float sign(float a) { return a >= 0 ? 1.0f : -1.0f; }

static inline float selectf(float a, float b, float c)
{
    return a >= 0 ? b : c;
}

static inline float invSqrt(float a) { return 1.0f / sqrtf(a); }

static inline float clamp(float a, float min, float max)
{
    float z = a;
    if (a < min)
        z = min;
    else if (a > max)
        z = max;
    return z;
}

static inline float clamp01(float a)
{
    float z = a;
    if (a < 0.0f)
        z = 0.0f;
    else if (a > 1.0f)
        z = 1.0f;
    return z;
}

static inline float lerp01(float a, float b, float t)
{
    return a + (b - a) * clamp01(t);
}

static inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

static inline char nearEq(float a, float b)
{
    return (char)(fabsf(b - a) <= EPSILON);
}

static inline char near0(float a) { return nearEq(a, 0); }

static inline float repeat(float t, float length)
{
    return clamp(t - floorf(t / length) * length, 0.0f, length);
}

static inline float pingPong(float t, float length)
{
    return length - fabsf(repeat(t, length * 2.0f) - length);
}

static inline float invLerp(float a, float b, float value)
{
    return a != b ? clamp01((value - a) / (b - a)) : 0.0f;
}

static inline float slerp(float from, float to, float t)
{
    float t0 = clamp01(t);
    t0 = -2.0f * t0 * t0 * t0 + 3.0f * t0 * t0;
    return to * t0 + from * (1.0f - t0);
}

static inline float deltaAngle(float current, float target)
{
    float delta = repeat((target - current), 360.0f);
    return delta > 180.0f ? delta - 360.0f : delta;
}

static inline float lerpAngle(float a, float b, float t)
{
    float delta = repeat((b - a), 360.0f);
    if (delta > 180)
        delta -= 360.0f;
    return a + delta * clamp01(t);
}

static inline float unwind(float a)
{
    while (a > 180.f)
        a -= 360.f;
    while (a < -180.f)
        a += 360.f;
    return a;
}

static inline float clampAxis(float a)
{
    a = fmodf(a, 360.0f);
    if (a < 0.0f)
        a += 360.0f;

    return a;
}

static inline float normAxis(float a)
{
    a = clampAxis(a);
    if (a > 180.0f)
        a -= 360.0f;

    return a;
}

static inline float clampAngle(float a, float min, float max)
{

    float md = clampAxis(max - min) * 0.5f;
    float rc = clampAxis(min + md);
    float dc = clampAxis(a - rc);

    if (dc > md)
        return normAxis(rc + md);
    else if (dc < -md)
        return normAxis(rc - md);

    return normAxis(a);
}

static inline int isFinite(float a)
{
    return finitef(a);
}

static inline float snap(float a, float size)
{
    return floorf((a + size * 0.5f) / size) * size;
}

static inline float moveTowards(float current, float target, float maxDelta)
{
    if (fabsf(target - current) <= maxDelta)
        return target;
    return current + sign(target - current) * maxDelta;
}

static inline float moveTowardsAngle(float current, float target, float maxDelta)
{
    float da = deltaAngle(current, target);
    if (-maxDelta < da && da < maxDelta)
        return target;
    return moveTowards(current, current + da, maxDelta);
}

static inline float smoothStep(float from, float to, float t)
{
    float t0 = clamp01(t);
    t0 = -2.0F * t0 * t0 * t0 + 3.0F * t0 * t0;
    return to * t0 + from * (1.0F - t0);
}

static inline float smoothDamp(float current, float target, float *currentVelocity,
                               float smoothTime,
                               float maxSpeed, float delta)
{
    float smoothTime0 = fmaxf(0.0001F, smoothTime);
    float omega = 2.0F / smoothTime0;
    float x = omega * delta;
    float exp = 1.0F / (1.0F + x + 0.48F * x * x + 0.235F * x * x * x);
    float change = current - target;
    float originalTo = target;

    float maxChange = maxSpeed * smoothTime0;
    change = clamp(change, -maxChange, maxChange);
    float target0 = current - change;
    float temp = (*currentVelocity + omega * change) * delta;
    *currentVelocity = (*currentVelocity - omega * temp) * exp;
    float output = target0 + (change + temp) * exp;

    if (originalTo - current > 0.0F == output > originalTo)
    {
        output = originalTo;
        *currentVelocity = (output - originalTo) / delta;
    }

    return output;
}

// tetra

static inline Tetrahedron tetrahedron(Vec3 a, Vec3 b, Vec3 c, Vec3 d)
{
    Tetrahedron t;
    t.a = a;
    t.b = b;
    t.c = c;
    t.d = d;
    return t;
}

// color

static inline Color color(float r, float g, float b, float a)
{
    Color c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}

static inline ColorHSL color_hslf(float r, float g, float b, float a)
{
    float max = fmaxf(r, fmaxf(g, b));
    float min = fminf(r, fminf(g, b));

    float h, s, l;
    h = s = l = (max + min) / 2.0f;

    if (max == min)
    {
        h = s = 0.0f;
    }
    else
    {
        float d = max - min;
        s = (l > 0.5) ? d / (2.0f - max - min) : d / (max + min);

        if (max == r)
        {
            h = (g - b) / d + ((g < b) ? 6.0f : 0.0f);
        }
        else if (max == g)
        {
            h = (b - r) / d + 2.0f;
        }
        else
        {
            h = (r - g) / d + 4.0f;
        }

        h *= 60.0f;
    }

    ColorHSL hsl;
    hsl.a = a;
    hsl.h = h;
    hsl.s = s;
    hsl.l = l;
    return hsl;
}

static inline ColorHSL color_hsl(Color rgb)
{
    return color_hslf(rgb.r, rgb.g, rgb.b, rgb.a);
}

static inline Color color_fromHSLf(float h, float s, float l, float a)
{
    float C = (1 - fabsf(2 * l - 1)) * s;
    float X = C * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    float m = l - C / 2.0f;

    float R, G, B;
    if (h >= 0 && h < 60)
    {
        R = C;
        G = X;
        B = 0;
    }
    else if (h >= 60 && h < 120)
    {
        R = X;
        G = C;
        B = 0;
    }
    else if (h >= 120 && h < 180)
    {
        R = 0;
        G = C;
        B = X;
    }
    else if (h >= 180 && h < 240)
    {
        R = 0;
        G = X;
        B = C;
    }
    else if (h >= 240 && h < 300)
    {
        R = X;
        G = 0;
        B = C;
    }
    else
    {
        R = C;
        G = 0;
        B = X;
    }
    Color rgb;
    rgb.a = a;
    rgb.r = R + m;
    rgb.g = G + m;
    rgb.b = B + m;
    return rgb;
}

static inline Color color_fromHSL(ColorHSL hsl)
{
    return color_fromHSLf(hsl.h, hsl.s, hsl.l, hsl.a);
}

static inline Color color_alpha(Color c, float a)
{
    c.a = a;
    return c;
}

static inline Color color_lerp(Color a, Color b, float d)
{
    a.a = lerp(a.a, b.a, d);
    a.r = lerp(a.r, b.r, d);
    a.g = lerp(a.g, b.g, d);
    a.b = lerp(a.b, b.b, d);
    return a;
}

static inline Color color_rand()
{
    Color a;
    a.a = 1;
    a.r = randf();
    a.g = randf();
    a.b = randf();
    return a;
}

static inline Color color_darken(Color rgba, float p)
{
    ColorHSL hsl = color_hsl(rgba);
    hsl.l = clamp(hsl.l - p, 0.0f, 1.0f);
    return color_fromHSL(hsl);
}

static inline Color color_lighten(Color rgba, float p)
{
    ColorHSL hsl = color_hsl(rgba);
    hsl.l = clamp(hsl.l + p, 0.0f, 1.0f);
    return color_fromHSL(hsl);
}

static inline Color color_lerp01(Color a, Color b, float d)
{
    a.a = lerp01(a.a, b.a, d);
    a.r = lerp01(a.r, b.r, d);
    a.g = lerp01(a.g, b.g, d);
    a.b = lerp01(a.b, b.b, d);
    return a;
}
// vec2

static inline Vec2 vec2(float x, float y)
{
    Vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

static inline Vec2 vec2f(float a)
{
    Vec2 v;
    v.x = a;
    v.y = a;
    return v;
}

static inline Vec2 vec2_neg(Vec2 a)
{
    a.x *= -1.0f;
    a.y *= -1.0f;
    return a;
}

static inline Vec2 vec2_add(Vec2 a, Vec2 b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

static inline Vec2 vec2_addf(Vec2 a, float b)
{
    a.x += b;
    a.y += b;
    return a;
}

static inline Vec2 vec2_sub(Vec2 a, Vec2 b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

static inline Vec2 vec2_subf(Vec2 a, float b)
{
    a.x -= b;
    a.y -= b;
    return a;
}

static inline Vec2 vec2_mulf(Vec2 a, float b)
{
    a.x *= b;
    a.y *= b;
    return a;
}

static inline Vec2 vec2_rot(Vec2 a, float d)
{
    float px = a.x;
    a.x = px * sind(d) + a.y * cosd(d);
    a.y = px * cosd(d) - a.y * sind(d);
    return a;
}

static inline float vec2_dot(Vec2 a, Vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

static inline float vec2_sqrMag(Vec2 a)
{
    return vec2_dot(a, a);
}

static inline float vec2_mag(Vec2 a)
{
    return sqrtf(vec2_dot(a, a));
}

static inline float vec2_dist(Vec2 a, Vec2 b)
{
    return vec2_mag(vec2_sub(a, b));
}

static inline char vec2_eq(Vec2 a, Vec2 b)
{
    return (char)(a.x == b.x && a.y == b.y);
}

static inline char vec2_eq0(Vec2 a)
{
    return (char)(a.x == 0.0f && a.y == 0.0f);
}

static inline char vec2_nearEq(Vec2 a, Vec2 b)
{
    return (char)(vec2_dist(a, b) < EPSILON);
}

static inline char vec2_near0(Vec2 a)
{
    return vec2_nearEq(a, vec2_zero);
}

static inline Vec2 vec2_min(Vec2 a, Vec2 b)
{
    a.x = fminf(a.x, b.x);
    a.y = fminf(a.y, b.y);
    return a;
}

static inline float vec2_minc(Vec2 a)
{
    return fminf(a.x, a.y);
}

static inline Vec2 vec2_max(Vec2 a, Vec2 b)
{
    a.x = fmaxf(a.x, b.x);
    a.y = fmaxf(a.y, b.y);
    return a;
}

static inline float vec2_maxc(Vec2 a)
{
    return fmaxf(a.x, a.y);
}

static inline Vec2 vec2_abs(Vec2 a)
{
    a.x = fabsf(a.x);
    a.y = fabsf(a.y);
    return a;
}

static inline float vec2_absMin(Vec2 a)
{
    return fminf(fabsf(a.x), fabsf(a.y));
}

static inline float vec2_absMax(Vec2 a)
{
    return fmaxf(fabsf(a.x), fabsf(a.y));
}

static inline Vec2 vec2_perpCw(Vec2 a)
{
    float ax = a.x;
    a.x = a.y;
    a.y = -ax;
    return a;
}

static inline Vec2 vec2_perpCcw(Vec2 a)
{

    float ax = a.x;
    a.x = -a.y;
    a.y = ax;
    return a;
}

static inline Vec2 vec2_norm(Vec2 a)
{
    float b = vec2_sqrMag(a);
    if (b == 1.0f)
        return a;
    if (b <= EPSILON2)
        return vec2_zero;

    float si = invSqrt(b);
    a.x *= si;
    a.y *= si;
    return a;
}

static inline Vec2 vec2_sign(Vec2 a)
{
    a.x = sign(a.x);
    a.y = sign(a.y);
    return a;
}

static inline Vec2 vec2_lerp(Vec2 a, Vec2 b, float dt)
{
    a.x = a.x + (b.x - a.x) * dt;
    a.y = a.y + (b.y - a.y) * dt;
    return a;
}

static inline Vec2 vec2_lerp01(Vec2 a, Vec2 b, float dt)
{
    return vec2_lerp(a, b, clamp01(dt));
}

static inline Vec2 vec2_reflect(Vec2 a, Vec2 n)
{
    return vec2_add(a, vec2_mulf(n, -2.0f * vec2_dot(n, a)));
}

static inline Vec2 vec2_projectNorm(Vec2 a, Vec2 n)
{
    return vec2_mulf(n, vec2_dot(a, n));
}

static inline Vec2 vec2_mirror(Vec2 a, Vec2 n)
{
    return vec2_sub(a, vec2_mulf(n, vec2_dot(a, n) * 2.0f));
}

static inline Vec2 vec2_snap(Vec2 a, float size)
{
    a.x = snap(a.x, size);
    a.y = snap(a.y, size);
    return a;
}

static inline Vec2 vec2_clamp(Vec2 a, Vec2 min, Vec2 max)
{
    a.x = clamp(a.x, -min.x, max.x);
    a.y = clamp(a.y, -min.y, max.y);
    return a;
}

static inline Vec2 vec2_clampf(Vec2 a, float min, float max)
{
    float sz = clamp(vec2_mag(a), min, max);
    return vec2_mulf(vec2_norm(a), sz);
}

static inline Vec2 vec2_rec(Vec2 a)
{
    if (nearEq(a.x, 0))
        a.x = MAX;
    else
        a.x = 1.0f / a.x;

    if (nearEq(a.y, 0))
        a.y = MAX;
    else
        a.y = 1.0f / a.y;

    return a;
}

static inline float vec2_angle(Vec2 a, Vec2 b)
{
    float de = vec2_sqrMag(a) * vec2_sqrMag(b);
    if (de <= EPSILON2)
        return 0.0f;
    return acosd(clamp(vec2_dot(a, b) * invSqrt(de), -1.0f, 1.0f));
}

static inline float vec2_cosAngle(Vec2 a, Vec2 b)
{
    a = vec2_norm(a);
    b = vec2_norm(b);
    return vec2_dot(a, b);
}

// vec3

static inline Vec3 vec3(float x, float y, float z)
{
    Vec3 a;
    a.x = x;
    a.y = y;
    a.z = z;
    return a;
}

static inline Vec3 vec3_rotXY(Vec3 a, float d)
{
    float px = a.x;
    a.x = px * cosd(d) + a.y * sind(d);
    a.y = -px * sind(d) + a.y * cosd(d);
    return a;
}

static inline Vec3 vec3_randv(Vec3 halfBound)
{
    Vec3 a;
    a.x = randf() * halfBound.x * 2.0f - halfBound.x;
    a.y = randf() * halfBound.y * 2.0f - halfBound.y;
    a.z = randf() * halfBound.z * 2.0f - halfBound.z;
    return a;
}

static inline Vec3 vec3_rand(float x, float y, float z)
{
    Vec3 a;
    a.x = randf() * x * 2.0f - x;
    a.y = randf() * y * 2.0f - y;
    a.z = randf() * z * 2.0f - z;
    return a;
}

static inline Vec3 vec3f(float a)
{
    Vec3 v;
    v.x = v.y = v.z = a;
    return v;
}

static inline Vec3 vec3_neg(Vec3 a)
{
    a.x *= -1.0f;
    a.y *= -1.0f;
    a.z *= -1.0f;
    return a;
}

static inline Vec3 vec3_add(Vec3 a, Vec3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}
static inline Vec3 vec3_center(Vec3 a, Vec3 b)
{
    a.x = (a.x + b.x) * 0.5f;
    a.y = (a.y + b.y) * 0.5f;
    a.z = (a.z + b.z) * 0.5f;
    return a;
}

static inline Vec3 vec3_addf(Vec3 a, float b)
{
    a.x += b;
    a.y += b;
    a.z += b;
    return a;
}

static inline Vec3 vec3_sub(Vec3 u, Vec3 v)
{
    u.x -= v.x;
    u.y -= v.y;
    u.z -= v.z;
    return u;
}

static inline Vec3 vec3_subf(Vec3 a, float b)
{
    a.x -= b;
    a.y -= b;
    a.z -= b;
    return a;
}

static inline Vec3 vec3_mulf(Vec3 a, float b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
    return a;
}

static inline float vec3_dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float vec3_sqrMag(Vec3 a)
{
    return vec3_dot(a, a);
}

static inline float vec3_mag(Vec3 a)
{
    return sqrtf(vec3_dot(a, a));
}

static inline float vec3_dist(Vec3 a, Vec3 b)
{
    return vec3_mag(vec3_sub(a, b));
}

static inline float vec3_dot2d(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y;
}

static inline float vec3_sqrMag2d(Vec3 a)
{
    return vec3_dot2d(a, a);
}

static inline float vec3_mag2d(Vec3 a)
{
    return sqrtf(vec3_dot2d(a, a));
}

static inline float vec3_dist2d(Vec3 a, Vec3 b)
{

    return vec3_mag2d(vec3_sub(a, b));
}

static inline char vec3_eq(Vec3 a, Vec3 b)
{
    return (char)(a.x == b.x && a.y == b.y && a.z == b.z);
}

static inline char vec3_eq0(Vec3 a)
{
    return (char)(a.x == 0 && a.y == 0 && a.z == 0);
}

static inline char vec3_nearEq(Vec3 a, Vec3 b)
{
    return (char)(vec3_dist(a, b) < EPSILON);
}

static inline char vec3_near0(Vec3 a)
{
    return vec3_nearEq(a, vec3_zero);
}

static inline Vec3 vec3_cross(Vec3 a, Vec3 b)
{
    Vec3 c;
    c.x = a.y * b.z - a.z * b.y;
    c.y = a.z * b.x - a.x * b.z;
    c.z = a.x * b.y - a.y * b.x;
    return c;
}

static inline Vec3 vec3_min(Vec3 a, Vec3 b)
{
    a.x = fminf(a.x, b.x);
    a.y = fminf(a.y, b.y);
    a.z = fminf(a.z, b.z);
    return a;
}

static inline float vec3_minc(Vec3 a)
{
    return fminf(fminf(a.x, a.y), a.z);
}

static inline Vec3 vec3_max(Vec3 a, Vec3 b)
{
    a.x = fmaxf(a.x, b.x);
    a.y = fmaxf(a.y, b.y);
    a.z = fmaxf(a.z, b.z);
    return a;
}

static inline float vec3_maxc(Vec3 a)
{
    return fmaxf(fmaxf(a.x, a.y), a.z);
}

static inline Vec3 vec3_abs(Vec3 a)
{
    a.x = fabsf(a.x);
    a.y = fabsf(a.y);
    a.z = fabsf(a.z);
    return a;
}

static inline float vec3_absMin(Vec3 a)
{
    return fminf(fminf(fabsf(a.x), fabsf(a.y)), fabsf(a.z));
}

static inline float vec3_absMax(Vec3 a)
{
    return fmaxf(fmaxf(fabsf(a.x), fabsf(a.y)), fabsf(a.z));
}

static inline Vec3 vec3_norm(Vec3 a)
{
    float s = vec3_sqrMag(a);
    if (s == 1.0f)
        return a;
    if (s <= EPSILON2)
        return vec3_zero;
    float si = invSqrt(s);
    a.x *= si;
    a.y *= si;
    a.z *= si;
    return a;
}

static inline Vec3 vec3_norm2d(Vec3 a)
{
    float s = vec3_sqrMag2d(a);
    if (s == 1)
    {
        if (a.z == 0)
            return a;
        a.z = 0;
        return a;
    }
    if (s <= EPSILON2)
        return vec3_zero;

    float si = invSqrt(s);
    a.x *= si;
    a.y *= si;
    a.z = 0;
    return a;
}

static inline Vec3 vec3_sign(Vec3 a)
{
    a.x = sign(a.x);
    a.y = sign(a.y);
    a.z = sign(a.z);
    return a;
}

static inline Vec3 vec3_lerp(Vec3 a, Vec3 b, float dt)
{
    a.x = a.x + (b.x - a.x) * dt;
    a.y = a.y + (b.y - a.y) * dt;
    a.z = a.z + (b.z - a.z) * dt;
    return a;
}

static inline Vec3 vec3_lerp01(Vec3 a, Vec3 b, float dt)
{
    return vec3_lerp(a, b, clamp01(dt));
}

static inline Vec3 vec3_moveTowards(Vec3 current, Vec3 target, float maxDelta)
{
    Vec3 toVector = vec3_sub(target, current);
    float dist = vec3_mag(toVector);
    if (dist <= maxDelta || dist < EPSILON)
        return target;
    return vec3_add(current, vec3_mulf(toVector, maxDelta / dist));
}

static inline Vec3 vec3_reflect(Vec3 a, Vec3 n)
{
    return vec3_add(a, vec3_mulf(n, -2.0f * vec3_dot(n, a)));
}

static inline Vec3 vec3_projection(Vec3 a)
{
    float rz = 1.0f / a.z;
    a.x = a.x * rz;
    a.y = a.y * rz;
    a.z = 0;
    return a;
}

static inline Vec3 vec3_project(Vec3 a, Vec3 b)
{
    float s = vec3_sqrMag(b);
    if (s <= EPSILON2)
        return vec3_zero;
    return vec3_mulf(b, vec3_dot(a, b) * invSqrt(s));
}

static inline Vec3 vec3_projectNorm(Vec3 a, Vec3 n)
{
    return vec3_mulf(n, vec3_dot(a, n));
}

static inline Vec3 vec3_mirror(Vec3 a, Vec3 n)
{
    return vec3_sub(a, vec3_mulf(n, vec3_dot(a, n) * 2.0f));
}

static inline Vec3 vec3_snap(Vec3 a, float size)
{
    a.x = snap(a.x, size);
    a.y = snap(a.y, size);
    a.z = snap(a.z, size);
    return a;
}

static inline Vec3 vec3_snapCube(Vec3 a, float size)
{
    float hs = size / 2;
    a.x = snap(-hs + a.x, size) + hs;
    a.y = snap(-hs + a.y, size) + hs;
    a.z = snap(-hs + a.z, size) + hs;
    return a;
}

static inline Vec3 vec3_snapCubeXY(Vec3 a, float size)
{
    float hs = size / 2;
    a.x = snap(-hs + a.x, size) + hs;
    a.y = snap(-hs + a.y, size) + hs;
    a.z = 0;
    return a;
}
static inline Vec3 vec3_clamp(Vec3 a, Vec3 min, Vec3 max)
{
    a.x = clamp(a.x, -min.x, max.x);
    a.y = clamp(a.y, -min.y, max.y);
    a.z = clamp(a.z, -min.z, max.z);
    return a;
}

static inline Vec3 vec3_clampf(Vec3 a, float min, float max)
{
    float sz = clamp(vec3_mag(a), min, max);
    return vec3_mulf(vec3_norm(a), sz);
}

static inline Vec3 vec3_clampf2d(Vec3 a, float min, float max)
{
    float z = a.z;
    a = vec3_mulf(vec3_norm(a), clamp(vec3_mag2d(a), min, max));
    a.z = z;
    return a;
}

static inline Vec3 vec3_rec(Vec3 a)
{
    if (nearEq(a.x, 0))
        a.x = MAX;
    else
        a.x = 1.0f / a.x;

    if (nearEq(a.y, 0))
        a.y = MAX;
    else
        a.y = 1.0f / a.y;

    if (nearEq(a.z, 0))
        a.z = MAX;
    else
        a.z = 1.0f / a.z;

    return a;
}

static inline float vec3_angle(Vec3 a, Vec3 b)
{
    float de = vec3_sqrMag(a) * vec3_sqrMag(b);
    if (de <= EPSILON2)
        return 0.0f;
    return acosd(clamp(vec3_dot(a, b) * invSqrt(de), -1.0f, 1.0f));
}

static inline float vec3_signedAngle(Vec3 a, Vec3 b, Vec3 axis)
{
    return vec3_angle(a, b) * sign(vec3_dot(axis, vec3_cross(a, b)));
}

static inline float vec3_cosAngle2d(Vec3 a, Vec3 b)
{
    a.z = 0;
    b.z = 0;
    a = vec3_norm(a);
    b = vec3_norm(b);
    return vec3_dot(a, b);
}

static inline Vec3 vec3_intersectPlane(Vec3 start, Vec3 end, Vec3 planeOrigin, Vec3 planeNormal)
{
    end = vec3_sub(end, start);
    return vec3_add(start, vec3_mulf(end, vec3_dot(vec3_sub(planeOrigin, start), planeNormal) / vec3_dot(end, planeNormal)));
}

// plane

static inline Plane plane(float x, float y, float z, float w)
{
    Plane p;
    p.x = x;
    p.y = y;
    p.z = z;
    p.w = w;
    return p;
}

//
static inline Edge edge(Vec3 a, Vec3 b)
{
    Edge e;
    e.a = a;
    e.b = b;
    return e;
}

// bbox

static inline BBox bbox(Vec3 a, Vec3 b)
{
    BBox bb;
    bb.min = a;
    bb.max = b;
    return bb;
}

static inline void bbox_vertices(BBox b, Vec3 *vertices)
{
    vertices[0] = b.min;
    vertices[1] = vec3(b.min.x, b.max.y, b.min.z);
    vertices[2] = vec3(b.max.x, b.max.y, b.min.z);
    vertices[3] = vec3(b.max.x, b.min.y, b.min.z);
    vertices[4] = vec3(b.min.x, b.min.y, b.max.z);
    vertices[5] = vec3(b.min.x, b.max.y, b.max.z);
    vertices[6] = b.max;
    vertices[7] = vec3(b.max.x, b.min.y, b.max.z);
}

static inline void bbox_edges(BBox b, Edge *edges)
{
    Vec3 vertices[8];
    bbox_vertices(b, vertices);

    edges[0] = edge(vertices[0], vertices[1]);
    edges[1] = edge(vertices[1], vertices[2]);
    edges[2] = edge(vertices[2], vertices[3]);
    edges[3] = edge(vertices[3], vertices[0]);

    edges[4] = edge(vertices[4], vertices[5]);
    edges[5] = edge(vertices[5], vertices[6]);
    edges[6] = edge(vertices[6], vertices[7]);
    edges[7] = edge(vertices[7], vertices[4]);

    edges[8] = edge(vertices[0], vertices[4]);
    edges[9] = edge(vertices[1], vertices[5]);
    edges[10] = edge(vertices[2], vertices[6]);
    edges[11] = edge(vertices[3], vertices[7]);
}

static inline BBox bbox_projectXY(BBox b)
{
    b.min.z = 0;
    b.max.z = 0;
    return b;
}

static inline BBox bbox_projectXZ(BBox b)
{
    b.min.y = 0;
    b.max.y = 0;
    return b;
}

static inline BBox bbox_projectYZ(BBox b)
{
    b.min.x = 0;
    b.max.x = 0;
    return b;
}

static inline BBox bbox_projectAxis(BBox b, int axis)
{
    if (axis == 0)
    {
        b.min.x = 0;
        b.max.x = 0;
    }
    else if (axis == 1)
    {
        b.min.y = 0;
        b.max.y = 0;
    }
    else if (axis == 2)
    {
        b.min.z = 0;
        b.max.z = 0;
    }
    return b;
}

static inline float bbox_depth(BBox b)
{
    return b.max.x - b.min.x;
}

static inline float bbox_height(BBox b)
{
    return b.max.z - b.min.z;
}

static inline float bbox_width(BBox b)
{
    return b.max.y - b.min.y;
}

static inline Vec3 bbox_center(BBox b)
{
    Vec3 a;
    a.x = (b.max.x + b.min.x) * 0.5f;
    a.y = (b.max.y + b.min.y) * 0.5f;
    a.z = (b.max.z + b.min.z) * 0.5f;
    return a;
}

static inline Vec3 bbox_size(BBox b)
{
    Vec3 a;
    a.x = (b.max.x - b.min.x);
    a.y = (b.max.y - b.min.y);
    a.z = (b.max.z - b.min.z);
    return a;
}

static inline BBox bbox_snap(BBox b, float s)
{
    Vec3 a;
    b.min = vec3_snap(b.min, s);
    b.max = vec3_snap(b.max, s);
    return b;
}

static inline BBox bbox_snapCube(BBox b, float s)
{
    Vec3 a;
    b.min = vec3_snapCube(b.min, s);
    b.max = vec3_snapCube(b.max, s);
    return b;
}

static inline float bbox_area(BBox b)
{
    return fmaxf(0, b.max.x - b.min.x) * fmaxf(0, b.max.y - b.min.y) * fmaxf(0, b.max.z - b.min.z);
}

static inline float bbox_margin(BBox b)
{
    return fmaxf(0, b.max.x - b.min.x) + fmaxf(0, b.max.y - b.min.y) + fmaxf(0, b.max.z - b.min.z);
}

static inline BBox bbox_expand(BBox b, float offset)
{
    b.min.x -= offset;
    b.min.y -= offset;
    b.min.z -= offset;
    b.max.x += offset;
    b.max.y += offset;
    b.max.z += offset;
    return b;
}

static inline BBox bbox_extend(BBox a, BBox b)
{
    a.min = vec3_min(a.min, b.min);
    a.max = vec3_max(a.max, b.max);
    return a;
}

static inline BBox bbox_intersection(BBox a, BBox b)
{
    a.min = vec3_max(a.min, b.min);
    a.max = vec3_min(a.max, b.max);
    return a;
}

static inline int bbox_contains(BBox a, BBox b)
{
    return a.min.x <= b.min.x &&
           a.min.y <= b.min.y &&
           a.min.z <= b.min.z &&
           a.max.x >= b.max.x &&
           a.max.y >= b.max.y &&
           a.max.z >= b.max.z;
}

static inline int bbox_containsPoint(BBox a, Vec3 b)
{
    return a.min.x <= b.x &&
           a.min.y <= b.y &&
           a.min.z <= b.z &&
           a.max.x >= b.x &&
           a.max.y >= b.y &&
           a.max.z >= b.z;
}

static inline int bbox_intersects(BBox a, BBox b)
{
    return a.min.x <= b.max.x &&
           a.min.y <= b.max.y &&
           a.min.z <= b.max.z &&
           a.max.x >= b.min.x &&
           a.max.y >= b.min.y &&
           a.max.z >= b.min.z;
}

static inline float axisDist(float p, float min, float max)
{
    return p < min ? min - p : p > max ? p - max
                                       : 0;
}

static inline float bbox_distance(BBox a, Vec3 b)
{
    float dX = axisDist(b.x, a.min.x, a.max.x);
    float dY = axisDist(b.y, a.min.y, a.max.y);
    float dZ = axisDist(b.z, a.min.z, a.max.z);
    return sqrtf(dX * dX + dY * dY + dZ * dZ);
}

static inline int bbox_isPlane(BBox b)
{
    return near0(bbox_width(b)) || near0(bbox_height(b)) || near0(bbox_depth(b));
}

static inline int bbox_isEdge(BBox b)
{
    return (near0(bbox_width(b)) && near0(bbox_height(b))) ||
           (near0(bbox_height(b)) && near0(bbox_depth(b))) ||
           (near0(bbox_width(b)) && near0(bbox_depth(b)));
}

static inline int bbox_isPoint(BBox b)
{
    return near0(bbox_width(b)) && near0(bbox_height(b)) && near0(bbox_depth(b));
}

static inline Vec3 bbox_planeDirection(BBox b)
{

    if (near0(bbox_width(b)))
        return vec3_right;
    if (near0(bbox_height(b)))
        return vec3_up;
    if (near0(bbox_depth(b)))
        return vec3_forward;
    return vec3_zero;
}

static inline BBox bbox_calculate(Vec3 *arr, int n)
{
    if (n == 0)
        return bbox_zero;
    Vec3 min = arr[0];
    Vec3 max = arr[0];
    for (int i = 1; i < n; i++)
    {
        min = vec3_min(min, arr[i]);
        max = vec3_max(max, arr[i]);
    }
    BBox b;
    b.min = min;
    b.max = max;
    return b;
}

// vec4

static inline Vec4 vec4(float x, float y, float z, float w)
{
    Vec4 a;
    a.x = x;
    a.y = y;
    a.z = z;
    a.w = w;
    return a;
}

// rot

static inline Rot rot(float pitch, float yaw, float roll)
{
    Rot r;
    r.pitch = pitch;
    r.yaw = yaw;
    r.roll = roll;
    return r;
}

static inline char rot_eq(Rot a, Rot b)
{
    return (char)(a.pitch == b.pitch && a.yaw == b.yaw && a.roll == b.roll);
}

static inline char rot_eq0(Rot a)
{
    return (char)(a.pitch == 0 && a.yaw == 0 && a.roll == 0);
}

static inline char rot_nearEq(Rot a, Rot b)
{
    return (char)(nearEq(a.pitch, b.pitch) &&
                  nearEq(a.yaw, b.yaw) & nearEq(a.roll, b.roll));
}

static inline char rot_near0(Rot a)
{
    return rot_nearEq(a, rot_zero);
}

static inline Rot rot_add(Rot a, Rot b)
{
    a.pitch += b.pitch;
    a.yaw += b.yaw;
    a.roll += b.roll;
    return a;
}

static inline Rot rot_addf(Rot a, float b)
{
    a.pitch += b;
    a.yaw += b;
    a.roll += b;
    return a;
}

static inline Rot rot_sub(Rot u, Rot v)
{
    u.pitch -= v.pitch;
    u.yaw -= v.yaw;
    u.roll -= v.roll;
    return u;
}

static inline Rot rot_subf(Rot a, float b)
{
    a.pitch -= b;
    a.yaw -= b;
    a.roll -= b;
    return a;
}

static inline Rot rot_mulf(Rot a, float b)
{
    a.pitch *= b;
    a.yaw *= b;
    a.roll *= b;
    return a;
}

static inline Rot rot_neg(Rot a)
{
    a.pitch *= -1.0f;
    a.yaw *= -1.0f;
    a.roll *= -1.0f;
    return a;
}

static inline Rot rot_snap(Rot a, float size)
{
    a.pitch = snap(a.pitch, size);
    a.yaw = snap(a.yaw, size);
    a.roll = snap(a.roll, size);
    return a;
}

static inline Rot rot_inv(Rot a)
{
    a.pitch *= -1;
    a.yaw += 180.0f;
    return a;
}

static inline Rot rot_clamp(Rot a)
{
    a.pitch = clampAxis(a.pitch);
    a.yaw = clampAxis(a.yaw);
    a.roll = clampAxis(a.roll);
    return a;
}

static inline char rot_nan(Rot a)
{
    return (char)(!isFinite(a.pitch) || !isFinite(a.yaw) || !isFinite(a.roll));
}

static inline Rot rot_norm(Rot a)
{
    a.pitch = normAxis(a.pitch);
    a.yaw = normAxis(a.yaw);
    a.roll = normAxis(a.roll);
    return a;
}

static inline Rot rot_deNorm(Rot a)
{
    a.pitch = clampAxis(a.pitch);
    a.yaw = clampAxis(a.yaw);
    a.roll = clampAxis(a.roll);
    return a;
}

static inline float rot_dist(Rot a, Rot b)
{
    return fabsf(a.pitch - b.pitch) + fabsf(a.yaw - b.yaw) +
           fabsf(a.roll - b.roll);
}

static inline Rot rot_eqv(Rot a)
{
    a.pitch = 180 - a.pitch;
    a.yaw = 180 + a.yaw;
    a.roll = 180 + a.roll;
    return a;
}

static inline Rot rot_lerp(Rot a, Rot b, float dt)
{
    Rot n = rot_norm(rot_sub(b, a));
    a.pitch = a.pitch + n.pitch * dt;
    a.yaw = a.yaw + n.yaw * dt;
    a.roll = a.roll + n.roll * dt;
    return a;
}

static inline Rot rot_lerp01(Rot a, Rot b, float dt)
{
    return rot_lerp(a, b, clamp01(dt));
}

static inline Rot rot_rlerp(Rot a, Rot b, float dt)
{
    a.pitch = a.pitch + (b.pitch - a.pitch) * dt;
    a.yaw = a.yaw + (b.yaw - a.yaw) * dt;
    a.roll = a.roll + (b.roll - a.roll) * dt;
    return rot_norm(a);
}

static inline Mat4 rot_matrix(Rot a, Vec3 origin)
{
    float cp = cosd(a.pitch);
    float sp = sind(a.pitch);
    float cy = cosd(a.yaw);
    float sy = sind(a.yaw);
    float cr = cosd(a.roll);
    float sr = sind(a.roll);
    return (Mat4){
        cp * cy, cp * sy, sp, 0.0f,
        sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, -sr * cp, 0.0f,
        -(cr * sp * cy + sr * sy), cy * sr - cr * sp * sy, cr * cp, 0.0f,
        origin.x, origin.y, origin.z, 1.0f};
}

static inline Vec3 rot_forward(Rot a)
{
    float pitch = fmodf(a.pitch, 360.0f);
    float yaw = fmodf(a.yaw, 360.0f);
    float cp = cosd(pitch);
    float sp = sind(pitch);
    float cy = cosd(yaw);
    float sy = sind(yaw);
    Vec3 v;
    v.x = cp * cy;
    v.y = cp * sy;
    v.z = sp;
    return v;
}

static inline Quat rot_quat(Rot a)
{
    float pitch = fmodf(a.pitch, 360.0f);
    float yaw = fmodf(a.yaw, 360.0f);
    float roll = fmodf(a.roll, 360.0f);

    float cp = cosd(pitch * 0.5f);
    float sp = sind(pitch * 0.5f);
    float cy = cosd(yaw * 0.5f);
    float sy = sind(yaw * 0.5f);
    float cr = cosd(roll * 0.5f);
    float sr = sind(roll * 0.5f);

    Quat q;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = -cr * cp * sy + sr * sp * cy;
    q.w = cr * cp * cy + sr * sp * sy;

    return q;
}

// quat

static inline Quat quat(float pitch, float yaw, float roll)
{
    Rot r;
    r.pitch = pitch;
    r.yaw = yaw;
    r.roll = roll;
    return rot_quat(r);
}

static inline Quat quat_mul(Quat a, Quat b)
{

    Quat q;
    q.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    q.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    q.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
    q.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    return q;
}

static inline Quat quat_mulf(Quat a, float b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
    a.w *= b;
    return a;
}

static inline float quat_dot(Quat a, Quat b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

static inline float quat_sqrmagnitude(Quat a) { return quat_dot(a, a); }

static inline float quat_magnitude(Quat a)
{
    return sqrtf(quat_sqrmagnitude(a));
}

static inline Quat quat_normalize(Quat a)
{
    float b = quat_sqrmagnitude(a);
    if (b <= EPSILON2)
        return quat_identity;
    float si = invSqrt(b);
    a.x *= si;
    a.y *= si;
    a.z *= si;
    a.w *= si;
    return a;
}

static inline Vec3 quat_unrotate(Quat q, Vec3 b)
{
    Vec3 a = {-q.x, -q.y, -q.z};
    Vec3 cross = vec3_mulf(vec3_cross(a, b), 2.0f);
    b = vec3_add(b, vec3_mulf(cross, q.w));
    b = vec3_add(b, vec3_cross(a, cross));
    return b;
}

static inline Vec3 quat_rotate(Quat q, Vec3 b)
{
    Vec3 a = {q.x, q.y, q.z};
    Vec3 cross = vec3_mulf(vec3_cross(a, b), 2.0f);
    b = vec3_add(b, vec3_mulf(cross, -q.w));
    b = vec3_add(b, vec3_cross(a, cross));
    return b;
}

static inline Vec3 quat_forward(Quat q)
{
    return quat_rotate(q, vec3_forward);
}

static inline Vec3 quat_right(Quat q)
{
    return quat_rotate(q, vec3_right);
}

static inline Vec3 quat_up(Quat q)
{
    return quat_rotate(q, vec3_up);
}

// mat4

static inline Mat4 mat4(float a)
{

    Mat4 m = {{
        {a, 0, 0, 0},
        {0, a, 0, 0},
        {0, 0, a, 0},
        {0, 0, 0, a},
    }};
    return m;
}

static inline Mat4 mat4_tran(Mat4 a)
{
    Mat4 m;
    m.m[0][0] = a.m[0][0];
    m.m[0][1] = a.m[1][0];
    m.m[0][2] = a.m[2][0];
    m.m[0][3] = a.m[3][0];
    m.m[1][0] = a.m[0][1];
    m.m[1][1] = a.m[1][1];
    m.m[1][2] = a.m[2][1];
    m.m[1][3] = a.m[3][1];
    m.m[2][0] = a.m[0][2];
    m.m[2][1] = a.m[1][2];
    m.m[2][2] = a.m[2][2];
    m.m[2][3] = a.m[3][2];
    m.m[3][0] = a.m[0][3];
    m.m[3][1] = a.m[1][3];
    m.m[3][2] = a.m[2][3];
    m.m[3][3] = a.m[3][3];
    return m;
}

static inline Mat4 mat4_mul(Mat4 a, Mat4 b)
{

    Mat4 m;
#if defined(USE_AVX_256) || defined(USE_AVX_128)
    __m128 tmp, sum;
    __m128 b0 = _mm_loadu_ps(b.m[0]);
    __m128 b1 = _mm_loadu_ps(b.m[1]);
    __m128 b2 = _mm_loadu_ps(b.m[2]);
    __m128 b3 = _mm_loadu_ps(b.m[3]);

    tmp = _mm_set_ps1(a.m[0][0]);
    sum = _mm_mul_ps(tmp, b0);
    tmp = _mm_set_ps1(a.m[0][1]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b1), sum);
    tmp = _mm_set_ps1(a.m[0][2]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b2), sum);
    tmp = _mm_set_ps1(a.m[0][3]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b3), sum);
    _mm_storeu_ps(m.m[0], sum);

    tmp = _mm_set_ps1(a.m[1][0]);
    sum = _mm_mul_ps(tmp, b0);
    tmp = _mm_set_ps1(a.m[1][1]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b1), sum);
    tmp = _mm_set_ps1(a.m[1][2]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b2), sum);
    tmp = _mm_set_ps1(a.m[1][3]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b3), sum);
    _mm_storeu_ps(m.m[1], sum);

    tmp = _mm_set_ps1(a.m[2][0]);
    sum = _mm_mul_ps(tmp, b0);
    tmp = _mm_set_ps1(a.m[2][1]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b1), sum);
    tmp = _mm_set_ps1(a.m[2][2]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b2), sum);
    tmp = _mm_set_ps1(a.m[2][3]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b3), sum);
    _mm_storeu_ps(m.m[2], sum);

    tmp = _mm_set_ps1(a.m[3][0]);
    sum = _mm_mul_ps(tmp, b0);
    tmp = _mm_set_ps1(a.m[3][1]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b1), sum);
    tmp = _mm_set_ps1(a.m[3][2]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b2), sum);
    tmp = _mm_set_ps1(a.m[3][3]);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b3), sum);
    _mm_storeu_ps(m.m[3], sum);
#else
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            m.m[i][j] = 0;
            for (int k = 0; k < 4; ++k)
            {
                m.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
#endif
    return m;
}

static inline Mat4 mat4_mul3(Mat4 a, Mat4 b, Mat4 c)
{
    return mat4_mul(mat4_mul(a, b), c);
}

static inline Mat4 mat4_add(Mat4 a, Mat4 b)
{
#if defined(USE_AVX_256)
    __m256 a0, b0;
    a0 = _mm256_loadu_ps(a.m[0]);
    b0 = _mm256_loadu_ps(b.m[0]);
    a0 = _mm256_add_ps(a0, b0);
    _mm256_storeu_ps(a.m[0], a0);

    a0 = _mm256_loadu_ps(a.m[2]);
    b0 = _mm256_loadu_ps(b.m[2]);
    a0 = _mm256_add_ps(a0, b0);
    _mm256_storeu_ps(a.m[2], a0);
#elif defined(USE_AVX_128)
    __m128 a0, b0;
    a0 = _mm_loadu_ps(a.m[0]);
    b0 = _mm_loadu_ps(b.m[0]);
    a0 = _mm_add_ps(a0, b0);
    _mm_storeu_ps(a.m[0], a0);
    a0 = _mm_loadu_ps(a.m[1]);
    b0 = _mm_loadu_ps(b.m[1]);
    a0 = _mm_add_ps(a0, b0);
    _mm_storeu_ps(a.m[1], a0);
    a0 = _mm_loadu_ps(a.m[2]);
    b0 = _mm_loadu_ps(b.m[2]);
    a0 = _mm_add_ps(a0, b0);
    _mm_storeu_ps(a.m[2], a0);
    a0 = _mm_loadu_ps(a.m[3]);
    b0 = _mm_loadu_ps(b.m[3]);
    a0 = _mm_add_ps(a0, b0);
    _mm_storeu_ps(a.m[3], a0);
#else
    for (int j = 0; j < 4; j++)
        for (int i = 0; i < 4; i++)
            a.m[j][i] += b.m[j][i];
#endif
    return a;
}

static inline Mat4 mat4_sub(Mat4 a, Mat4 b)
{
#if defined(USE_AVX_256)
    __m256 a0, b0;
    a0 = _mm256_loadu_ps(a.m[0]);
    b0 = _mm256_loadu_ps(b.m[0]);
    a0 = _mm256_sub_ps(a0, b0);
    _mm256_storeu_ps(a.m[0], a0);
    a0 = _mm256_loadu_ps(a.m[2]);
    b0 = _mm256_loadu_ps(b.m[2]);
    a0 = _mm256_sub_ps(a0, b0);
    _mm256_storeu_ps(a.m[2], a0);
#elif defined(USE_AVX_128)
    __m128 a0, b0;
    a0 = _mm_loadu_ps(a.m[0]);
    b0 = _mm_loadu_ps(b.m[0]);
    a0 = _mm_sub_ps(a0, b0);
    _mm_storeu_ps(a.m[0], a0);
    a0 = _mm_loadu_ps(a.m[1]);
    b0 = _mm_loadu_ps(b.m[1]);
    a0 = _mm_sub_ps(a0, b0);
    _mm_storeu_ps(a.m[1], a0);
    a0 = _mm_loadu_ps(a.m[2]);
    b0 = _mm_loadu_ps(b.m[2]);
    a0 = _mm_sub_ps(a0, b0);
    _mm_storeu_ps(a.m[2], a0);
    a0 = _mm_loadu_ps(a.m[3]);
    b0 = _mm_loadu_ps(b.m[3]);
    a0 = _mm_sub_ps(a0, b0);
    _mm_storeu_ps(a.m[3], a0);
#else
    for (int j = 0; j < 4; j++)
        for (int i = 0; i < 4; i++)
            a.m[j][i] -= b.m[j][i];
#endif
    return a;
}

static inline Mat4 mat4_mulf(Mat4 a, float b)
{
#if defined(USE_AVX_256)
    __m256 a0, b0;
    b0 = _mm256_set1_ps(b);
    a0 = _mm256_loadu_ps(a.m[0]);
    a0 = _mm256_mul_ps(a0, b0);
    _mm256_storeu_ps(a.m[0], a0);
    a0 = _mm256_loadu_ps(a.m[2]);
    a0 = _mm256_mul_ps(a0, b0);
    _mm256_storeu_ps(a.m[2], a0);
#elif defined(USE_AVX_128)
    __m128 a0, b0;
    b0 = _mm_set1_ps(b);
    a0 = _mm_loadu_ps(a.m[0]);
    a0 = _mm_mul_ps(a0, b0);
    _mm_storeu_ps(a.m[0], a0);
    a0 = _mm_loadu_ps(a.m[1]);
    a0 = _mm_mul_ps(a0, b0);
    _mm_storeu_ps(a.m[1], a0);
    a0 = _mm_loadu_ps(a.m[2]);
    a0 = _mm_mul_ps(a0, b0);
    _mm_storeu_ps(a.m[2], a0);
    a0 = _mm_loadu_ps(a.m[3]);
    a0 = _mm_mul_ps(a0, b0);
    _mm_storeu_ps(a.m[3], a0);
#else
    for (int j = 0; j < 4; j++)
        for (int i = 0; i < 4; i++)
            a.m[j][i] *= b;
#endif
    return a;
}

static inline char mat4_eq(Mat4 a, Mat4 b)
{
    for (char j = 0; j < 4; j++)
        for (char i = 0; i < 4; i++)
            if (a.m[j][i] != b.m[j][i])
                return 0;
    return 1;
}

static inline char mat4_nearEq(Mat4 a, Mat4 b)
{

    for (char j = 0; j < 4; j++)
        for (char i = 0; i < 4; i++)
            if (!nearEq(a.m[j][i], b.m[j][i]))
                return 0;
    return 1;
}

static inline Vec4 mat4_mulv4(Mat4 a, Vec4 b)
{
#if defined(USE_AVX_256) || defined(USE_AVX_128)

    __m128 tmp, sum;
    __m128 b0 = _mm_loadu_ps(a.m[0]);
    __m128 b1 = _mm_loadu_ps(a.m[1]);
    __m128 b2 = _mm_loadu_ps(a.m[2]);
    __m128 b3 = _mm_loadu_ps(a.m[3]);

    tmp = _mm_set_ps1(b.x);
    sum = _mm_mul_ps(tmp, b0);
    tmp = _mm_set_ps1(b.y);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b1), sum);
    tmp = _mm_set_ps1(b.z);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b2), sum);
    tmp = _mm_set_ps1(b.w);
    sum = _mm_add_ps(_mm_mul_ps(tmp, b3), sum);
    _mm_storeu_ps((float *)(&b), sum);
    return b;

#else
#endif
    return b;
}

static inline Vec3 mat4_mulv3(Mat4 a, Vec3 b, float w)
{
    Vec4 v = {b.x, b.y, b.z, w};
    v = mat4_mulv4(a, v);
    b.x = v.x;
    b.y = v.y;
    b.z = v.z;
    return b;
}

static inline Mat4 mat4_scale(Vec3 a)
{
    Mat4 m;

    m.m[0][0] = a.x;
    m.m[0][1] = 0.0f;
    m.m[0][2] = 0.0f;
    m.m[0][3] = 0.0f;

    m.m[1][0] = 0.0f;
    m.m[1][1] = a.y;
    m.m[1][2] = 0.0f;
    m.m[1][3] = 0.0f;

    m.m[2][0] = 0.0f;
    m.m[2][1] = 0.0f;
    m.m[2][2] = a.z;
    m.m[2][3] = 0.0f;

    m.m[3][0] = 0.0f;
    m.m[3][1] = 0.0f;
    m.m[3][2] = 0.0f;
    m.m[3][3] = 1.0f;

    return m;
}

static inline Mat4 mat4_scalef(float a)
{

    Mat4 m;

    m.m[0][0] = a;
    m.m[0][1] = 0.0f;
    m.m[0][2] = 0.0f;
    m.m[0][3] = 0.0f;

    m.m[1][0] = 0.0f;
    m.m[1][1] = a;
    m.m[1][2] = 0.0f;
    m.m[1][3] = 0.0f;

    m.m[2][0] = 0.0f;
    m.m[2][1] = 0.0f;
    m.m[2][2] = a;
    m.m[2][3] = 0.0f;

    m.m[3][0] = 0.0f;
    m.m[3][1] = 0.0f;
    m.m[3][2] = 0.0f;
    m.m[3][3] = 1.0f;

    return m;
}

static inline char mat4_containsNaN(Mat4 a)
{
    for (char j = 0; j < 4; j++)
        for (char i = 0; i < 4; i++)
            if (!finite(a.m[j][i]))
                return 0;
    return 1;
}

static inline Mat4 mat4_origin(Vec3 a)
{
    Mat4 m;
    m.m[0][0] = 1.0f;
    m.m[0][1] = 0.0f;
    m.m[0][2] = 0.0f;
    m.m[0][3] = 0.0f;

    m.m[1][0] = 0.0f;
    m.m[1][1] = 1.0f;
    m.m[1][2] = 0.0f;
    m.m[1][3] = 0.0f;

    m.m[2][0] = 0.0f;
    m.m[2][1] = 0.0f;
    m.m[2][2] = 1.0f;
    m.m[2][3] = 0.0f;

    m.m[3][0] = a.x;
    m.m[3][1] = a.y;
    m.m[3][2] = a.z;
    m.m[3][3] = 1.0f;

    return m;
}

static inline Vec3 mat4_axis(const Mat4 *a, UnitAxisEnum ax)
{
    Vec3 b;
    switch (ax)
    {
    case UNIT_AXIS_X:
        b.x = a->m[0][0];
        b.y = a->m[0][1];
        b.z = a->m[0][2];
        return b;

    case UNIT_AXIS_Y:
        b.x = a->m[1][0];
        b.y = a->m[1][1];
        b.z = a->m[1][2];
        return b;

    case UNIT_AXIS_Z:
        b.x = a->m[2][0];
        b.y = a->m[2][1];
        b.z = a->m[2][2];
        return b;
    }
    return vec3_zero;
}

static inline void mat4_axes(const Mat4 *a, Vec3 *ax, Vec3 *ay, Vec3 *az)
{
    ax->x = a->m[0][0];
    ax->y = a->m[0][1];
    ax->z = a->m[0][2];
    ay->x = a->m[1][0];
    ay->y = a->m[1][1];
    ay->z = a->m[1][2];
    az->x = a->m[2][0];
    az->y = a->m[2][1];
    az->z = a->m[2][2];
}

static inline Mat4
mat4_orthographic(float left, float right, float bottom, float top, float nr,
                  float far)
{
    Mat4 m;

    float fmd = far - nr;
    float fpd = far + nr;
    float tmb = top - bottom;
    float tpb = top + bottom;
    float rml = right - left;
    float rpl = right + left;

    m.m[0][0] = 2.0f / rml;
    m.m[0][1] = 0.0f;
    m.m[0][2] = 0.0f;
    m.m[0][3] = 0.0f;

    m.m[1][0] = 0.0f;
    m.m[1][1] = 2.0f / tmb;
    m.m[1][2] = 0.0f;
    m.m[1][3] = 0.0f;

    m.m[2][0] = 0.0f;
    m.m[2][1] = 0.0f;
    m.m[2][2] = 1.0f / fmd;
    m.m[2][3] = 0.0f;

    m.m[3][0] = -rpl / rml;
    m.m[3][1] = -tpb / tmb;
    m.m[3][2] = -fpd / fmd;
    m.m[3][3] = 1.0f;

    return m;
}

static inline Mat4
mat4_perspective(float fov, float aspect, float nr, float fr)
{
    float t = tand(fov * 0.5f);
    float inv = 1.0f / (fr - nr);

    Mat4 m;
    m.m[0][0] = 1.0f / t;
    m.m[0][1] = 0.0f;
    m.m[0][2] = 0.0f;
    m.m[0][3] = 0.0f;

    m.m[1][0] = 0.0f;
    m.m[1][1] = aspect / t;
    m.m[1][2] = 0.0f;
    m.m[1][3] = 0.0f;

    m.m[2][0] = 0.0f;
    m.m[2][1] = 0.0f;
    m.m[2][2] = (fr + nr) * inv;
    m.m[2][3] = 1.0f;

    m.m[3][0] = 0.0f;
    m.m[3][1] = 0.0f;
    m.m[3][2] = -(2 * nr * fr) * inv;
    m.m[3][3] = 0.0f;

    return m;
}

static inline Mat4 mat4_lookAt(Vec3 eye, Vec3 center, Vec3 up)
{
    Mat4 m;
    Vec3 zaxis = vec3_norm(vec3_sub(center, eye));
    Vec3 xaxis = vec3_norm(vec3_cross(up, zaxis));
    Vec3 yaxis = vec3_cross(zaxis, xaxis);

    m.m[0][0] = xaxis.x;
    m.m[1][0] = xaxis.y;
    m.m[2][0] = xaxis.z;
    m.m[3][0] = -vec3_dot(eye, xaxis);
    m.m[0][1] = yaxis.x;
    m.m[1][1] = yaxis.y;
    m.m[2][1] = yaxis.z;
    m.m[3][1] = -vec3_dot(eye, yaxis);
    m.m[0][2] = zaxis.x;
    m.m[1][2] = zaxis.y;
    m.m[2][2] = zaxis.z;
    m.m[3][2] = -vec3_dot(eye, zaxis);
    m.m[0][3] = 0.0f;
    m.m[1][3] = 0.0f;
    m.m[2][3] = 0.0f;
    m.m[3][3] = 1.0f;

    return m;
}

static inline Vec3 rot_right(Rot a)
{
    Mat4 m = rot_matrix(a, vec3_zero);
    return mat4_axis(&m, UNIT_AXIS_Y);
}

static inline Vec3 rot_up(Rot a)
{
    Mat4 m = rot_matrix(a, vec3_zero);
    return mat4_axis(&m, UNIT_AXIS_Z);
}

static inline Vec3 rot_rotate(Rot r, Vec3 b)
{
    return mat4_mulv3(rot_matrix(r, vec3_zero), b, 1);
}

static inline Vec3 rot_unrotate(Rot r, Vec3 b)
{
    return mat4_mulv3(mat4_tran(rot_matrix(r, vec3_zero)), b, 1);
}

static inline Mat4 mat4_vec3(Vec3 xa, Vec3 ya, Vec3 za, Vec3 wa)
{
    Mat4 m;
    m.m[0][0] = xa.x;
    m.m[0][1] = xa.y;
    m.m[0][2] = xa.z;
    m.m[0][3] = 0.0f;

    m.m[1][0] = ya.x;
    m.m[1][1] = ya.y;
    m.m[1][2] = ya.z;
    m.m[1][3] = 0.0f;

    m.m[2][0] = za.x;
    m.m[2][1] = za.y;
    m.m[2][2] = za.z;
    m.m[2][3] = 0.0f;

    m.m[3][0] = wa.x;
    m.m[3][1] = wa.y;
    m.m[3][2] = wa.z;
    m.m[3][3] = 1.0f;
    return m;
}

static inline Mat4 mat4_plane2(Plane a, Plane b, Plane c, Plane d)
{
    Mat4 m;
    m.m[0][0] = a.x;
    m.m[0][1] = a.y;
    m.m[0][2] = a.z;
    m.m[0][3] = a.w;
    m.m[1][0] = b.x;
    m.m[1][1] = b.y;
    m.m[1][2] = b.z;
    m.m[1][3] = b.w;
    m.m[2][0] = c.x;
    m.m[2][1] = c.y;
    m.m[2][2] = c.z;
    m.m[2][3] = c.w;
    m.m[3][0] = d.x;
    m.m[3][1] = d.y;
    m.m[3][2] = d.z;
    m.m[3][3] = d.w;
    return m;
}

static inline Rot mat4_rot(Mat4 m)
{
    Vec3 ax, ay, az;
    mat4_axes(&m, &ax, &ay, &az);

    Rot r = rot(
        atan2d(ax.z, sqrtf(square(ax.x) + square(ax.y))),
        atan2d(ax.y, ax.x),
        0);

    Mat4 m2 = rot_matrix(r, vec3_zero);
    Vec3 say = mat4_axis(&m2, UNIT_AXIS_Y);
    r.roll = atan2d(vec3_dot(az, say), vec3_dot(ay, say));
    return r;
}

static inline Mat4 mat4_invRot(Rot a)
{
    float cy = cosd(a.yaw);
    float sy = sind(a.yaw);
    float cp = cosd(a.pitch);
    float sp = sind(a.pitch);
    float cr = cosd(a.roll);
    float sr = sind(a.roll);
    Mat4 ma = {{
        // z
        {+cy, -sy, 0.f, 0.f},
        {+sy, +cy, 0.f, 0.f},
        {0.f, 0.f, 1.f, 0.f},
        {0.f, 0.f, 0.f, 1.f},
    }};
    Mat4 mb = {{
        // y
        {+cp, 0.f, -sp, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {+sp, 0.f, +cp, 0.f},
        {0.f, 0.f, 0.f, 1.f},
    }};
    Mat4 mc = {{
        // x
        {1.f, 0.f, 0.f, 0.f},
        {0.f, +cr, -sr, 0.f},
        {0.f, +sr, +cr, 0.f},
        {0.f, 0.f, 0.f, 1.f},
    }};
    Mat4 m = mat4_mul3(ma, mb, mc);

    return m;
}

static inline Mat4 mat4_fromX(Vec3 x)
{
    x = vec3_norm(x);
    Vec3 up = (fabsf(x.z) < (1.f - EPSILON)) ? vec3(0, 0, 1.0f) : vec3(1.0f, 0, 0);
    Vec3 y = vec3_norm(vec3_cross(up, x));
    Vec3 z = vec3_cross(x, y);
    return mat4_vec3(x, y, z, vec3_zero);
}

static inline Mat4 mat4_fromY(Vec3 y)
{
    y = vec3_norm(y);
    Vec3 up = (fabsf(y.z) < (1.f - EPSILON)) ? vec3(0, 0, 1.0f) : vec3(1.0f, 0, 0);
    Vec3 z = vec3_norm(vec3_cross(up, y));
    Vec3 x = vec3_cross(y, z);
    return mat4_vec3(x, y, z, vec3_zero);
}

static inline Mat4 mat4_fromZ(Vec3 z)
{
    z = vec3_norm(z);
    Vec3 up = (fabsf(z.z) < (1.f - EPSILON)) ? vec3(0, 0, 1.0f) : vec3(1.0f, 0, 0);
    Vec3 x = vec3_norm(vec3_cross(up, z));
    Vec3 y = vec3_cross(z, x);
    return mat4_vec3(x, y, z, vec3_zero);
}

static inline Mat4 mat4_view(Vec3 a, Rot b)
{
    Mat4 ma = {
        {{0, 0, 1, 0},
         {1, 0, 0, 0},
         {0, 1, 0, 0},
         {0, 0, 0, 1}}};
    return mat4_mul(
        mat4_origin(vec3_neg(a)),
        mat4_mul(mat4_invRot(b), ma));
}

static inline Mat4 mat4_transformf(Vec3 origin, float scale)
{
    Mat4 mt = {
        scale, 0, 0, 0,
        0, scale, 0, 0,
        0, 0, scale, 0,
        origin.x, origin.y, origin.z, 1};
    return mt;
}

static inline float mat4_det(Mat4 m)
{
    return m.m[0][0] * (m.m[1][1] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) -
                        m.m[2][1] * (m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2]) +
                        m.m[3][1] * (m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2])) -
           m.m[1][0] * (m.m[0][1] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) -
                        m.m[2][1] * (m.m[0][2] * m.m[3][3] - m.m[0][3] * m.m[3][2]) +
                        m.m[3][1] * (m.m[0][2] * m.m[2][3] - m.m[0][3] * m.m[2][2])) +
           m.m[2][0] * (m.m[0][1] * (m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2]) -
                        m.m[1][1] * (m.m[0][2] * m.m[3][3] - m.m[0][3] * m.m[3][2]) +
                        m.m[3][1] * (m.m[0][2] * m.m[1][3] - m.m[0][3] * m.m[1][2])) -
           m.m[3][0] * (m.m[0][1] * (m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2]) -
                        m.m[1][1] * (m.m[0][2] * m.m[2][3] - m.m[0][3] * m.m[2][2]) +
                        m.m[2][1] * (m.m[0][2] * m.m[1][3] - m.m[0][3] * m.m[1][2]));
}

static inline Mat4 mat4_inv(Mat4 m)
{

    float da[4];
    Mat4 tmp, res;

    tmp.m[0][0] = m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2];
    tmp.m[0][1] = m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2];
    tmp.m[0][2] = m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2];

    tmp.m[1][0] = m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2];
    tmp.m[1][1] = m.m[0][2] * m.m[3][3] - m.m[0][3] * m.m[3][2];
    tmp.m[1][2] = m.m[0][2] * m.m[2][3] - m.m[0][3] * m.m[2][2];

    tmp.m[2][0] = m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2];
    tmp.m[2][1] = m.m[0][2] * m.m[3][3] - m.m[0][3] * m.m[3][2];
    tmp.m[2][2] = m.m[0][2] * m.m[1][3] - m.m[0][3] * m.m[1][2];

    tmp.m[3][0] = m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2];
    tmp.m[3][1] = m.m[0][2] * m.m[2][3] - m.m[0][3] * m.m[2][2];
    tmp.m[3][2] = m.m[0][2] * m.m[1][3] - m.m[0][3] * m.m[1][2];

    da[0] = m.m[1][1] * tmp.m[0][0] - m.m[2][1] * tmp.m[0][1] +
            m.m[3][1] * tmp.m[0][2];
    da[1] = m.m[0][1] * tmp.m[1][0] - m.m[2][1] * tmp.m[1][1] +
            m.m[3][1] * tmp.m[1][2];
    da[2] = m.m[0][1] * tmp.m[2][0] - m.m[1][1] * tmp.m[2][1] +
            m.m[3][1] * tmp.m[2][2];
    da[3] = m.m[0][1] * tmp.m[3][0] - m.m[1][1] * tmp.m[3][1] +
            m.m[2][1] * tmp.m[3][2];

    const float det =
        m.m[0][0] * da[0] - m.m[1][0] * da[1] + m.m[2][0] * da[2] -
        m.m[3][0] * da[3];
    const float rdet = 1.0f / det;

    res.m[0][0] = rdet * da[0];
    res.m[0][1] = -rdet * da[1];
    res.m[0][2] = rdet * da[2];
    res.m[0][3] = -rdet * da[3];
    res.m[1][0] = -rdet * (m.m[1][0] * tmp.m[0][0] - m.m[2][0] * tmp.m[0][1] +
                           m.m[3][0] * tmp.m[0][2]);
    res.m[1][1] = rdet * (m.m[0][0] * tmp.m[1][0] - m.m[2][0] * tmp.m[1][1] +
                          m.m[3][0] * tmp.m[1][2]);
    res.m[1][2] = -rdet * (m.m[0][0] * tmp.m[2][0] - m.m[1][0] * tmp.m[2][1] +
                           m.m[3][0] * tmp.m[2][2]);
    res.m[1][3] = rdet * (m.m[0][0] * tmp.m[3][0] - m.m[1][0] * tmp.m[3][1] +
                          m.m[2][0] * tmp.m[3][2]);
    res.m[2][0] = rdet *
                  (m.m[1][0] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) -
                   m.m[2][0] * (m.m[1][1] * m.m[3][3] - m.m[1][3] * m.m[3][1]) +
                   m.m[3][0] * (m.m[1][1] * m.m[2][3] - m.m[1][3] * m.m[2][1]));
    res.m[2][1] = -rdet *
                  (m.m[0][0] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) -
                   m.m[2][0] * (m.m[0][1] * m.m[3][3] - m.m[0][3] * m.m[3][1]) +
                   m.m[3][0] * (m.m[0][1] * m.m[2][3] - m.m[0][3] * m.m[2][1]));
    res.m[2][2] = rdet *
                  (m.m[0][0] * (m.m[1][1] * m.m[3][3] - m.m[1][3] * m.m[3][1]) -
                   m.m[1][0] * (m.m[0][1] * m.m[3][3] - m.m[0][3] * m.m[3][1]) +
                   m.m[3][0] * (m.m[0][1] * m.m[1][3] - m.m[0][3] * m.m[1][1]));
    res.m[2][3] = -rdet *
                  (m.m[0][0] * (m.m[1][1] * m.m[2][3] - m.m[1][3] * m.m[2][1]) -
                   m.m[1][0] * (m.m[0][1] * m.m[2][3] - m.m[0][3] * m.m[2][1]) +
                   m.m[2][0] * (m.m[0][1] * m.m[1][3] - m.m[0][3] * m.m[1][1]));
    res.m[3][0] = -rdet *
                  (m.m[1][0] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1]) -
                   m.m[2][0] * (m.m[1][1] * m.m[3][2] - m.m[1][2] * m.m[3][1]) +
                   m.m[3][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]));
    res.m[3][1] = rdet *
                  (m.m[0][0] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1]) -
                   m.m[2][0] * (m.m[0][1] * m.m[3][2] - m.m[0][2] * m.m[3][1]) +
                   m.m[3][0] * (m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1]));
    res.m[3][2] = -rdet *
                  (m.m[0][0] * (m.m[1][1] * m.m[3][2] - m.m[1][2] * m.m[3][1]) -
                   m.m[1][0] * (m.m[0][1] * m.m[3][2] - m.m[0][2] * m.m[3][1]) +
                   m.m[3][0] * (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]));
    res.m[3][3] = rdet *
                  (m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) -
                   m.m[1][0] * (m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1]) +
                   m.m[2][0] * (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]));

    return res;
}

static inline Rot rot_lookAt(Vec3 a, Vec3 b, Vec3 up)
{
    Vec3 forward = vec3_norm(vec3_sub(b, a));
    return mat4_rot(mat4_fromX(forward));
}

//

static inline Transform transform(Vec3 pos, Rot r, Vec3 scale)
{
    Transform t;
    t.position = pos;
    t.rotation = r;
    t.scale = scale;
    return t;
}

//
static inline Ray ray(Vec3 origin, Vec3 direction)
{
    Ray r;
    r.origin = origin;
    r.direction = direction;
    return r;
}

static inline Ray ray_fromTo(Vec3 a, Vec3 b)
{
    Ray r;
    r.origin = a;
    r.direction = vec3_sub(b, a);
    return r;
}

static inline Ray ray_fromRot(Vec3 a, Rot b)
{
    Ray r;
    r.origin = a;
    r.direction = rot_forward(b);
    return r;
}

static inline Ray ray_scale(Ray r, float scale)
{
    r.direction = vec3_mulf(r.direction, scale);
    return r;
}

static inline Ray ray_move(Ray r, float distance)
{
    r.origin = vec3_add(r.origin, vec3_mulf(r.direction, distance));
    return r;
}

static inline char ray_hitSphere(Ray r, Sphere s, Vec3 *hit)
{
    Vec3 oc = vec3_sub(s.position, r.origin);
    float a = vec3_sqrMag(r.direction);
    float b = 2.0f * (oc.x * r.direction.x +
                      oc.y * r.direction.y +
                      oc.z * r.direction.z);
    float c = vec3_sqrMag(oc) - square(s.radius);
    float dis = square(b) - (4 * a * c);
    if (dis < 0)
        return 0;
    if (hit != NULL)
    {
        float t = -fminf((-b - sqrtf(dis)) / (2.0f * a),
                         (-b + sqrtf(dis)) / (2.0f * a));
        *hit = vec3_add(r.origin, vec3_mulf(r.direction, t));
    }
    return 1;
}

static inline char
ray_hitCircle(Ray r, Sphere s, Vec3 normal, Vec3 *hit)
{
    Vec3 v = vec3_sub(s.position, r.origin);
    float de = vec3_dot(r.direction, normal);
    if (de < EPSILON)
        return 0;

    float t = vec3_dot(v, normal) / de;
    Vec3 itp = vec3_add(r.origin, vec3_mulf(r.direction, t));
    float dist = vec3_mag(vec3_sub(itp, s.position));

    if (hit != NULL)
        *hit = vec3_add(r.origin, vec3_mulf(r.direction, t));
    return (char)(dist <= s.radius);
}

static inline char ray_hitBBox(Ray r, BBox b, Vec3 *hit)
{
    Vec3 re;
    re.x = 1.0f / r.direction.x;
    re.y = 1.0f / r.direction.y;
    re.z = 1.0f / r.direction.z;

    float t1 = (b.min.x - r.origin.x) * re.x;
    float t2 = (b.max.x - r.origin.x) * re.x;

    float t3 = (b.min.y - r.origin.y) * re.y;
    float t4 = (b.max.y - r.origin.y) * re.y;

    float t5 = (b.min.z - r.origin.z) * re.z;
    float t6 = (b.max.z - r.origin.z) * re.z;

    float int_min = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
    float int_max = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));

    if (int_max < 0)
        return 0;

    if (int_min > int_max)
        return 0;

    if (hit != NULL)
        *hit = vec3_add(r.origin, vec3_mulf(r.direction, int_max));

    return 1;
}

static inline char ray_hitTriangle(Ray r, Triangle tri, Vec3 *hit)
{

    Vec3 edge1 = vec3_sub(tri.b, tri.a);
    Vec3 edge2 = vec3_sub(tri.c, tri.a);
    Vec3 h = vec3_cross(r.direction, edge2);
    float a = vec3_dot(edge1, h);

    if (fabs(a) < EPSILON)
        return 0;

    float f = 1.0f / a;
    Vec3 s = vec3_sub(r.origin, tri.a);
    float u = f * vec3_dot(s, h);

    if (u < 0.0f || u > 1.0f)
        return 0;

    Vec3 q = vec3_cross(s, edge1);
    float v = f * vec3_dot(r.direction, q);

    if (v < 0.0f || u + v > 1.0f)
        return 0;

    float t = f * vec3_dot(edge2, q);

    if (t > EPSILON)
    {

        if (hit != NULL)
            *hit = vec3_add(r.origin, vec3_mulf(r.direction, t));
        return 1;
    }

    return 9;
}

static inline char ray_hitQuad(Ray r, Quad q, Vec3 *hit)
{
    if (ray_hitTriangle(r, (Triangle){q.a, q.b, q.c}, hit))
    {
        return 1;
    }
    if (ray_hitTriangle(r, (Triangle){q.a, q.c, q.d}, hit))
    {
        return 1;
    }
    return 0;
}

//

static inline int edge_eq(Edge a, Edge b)
{
    return (
        (vec3_eq(a.a, b.a) && vec3_eq(a.b, b.b)) ||
        (vec3_eq(a.a, b.b) && vec3_eq(a.b, b.a)));
}

static inline int edge_nearEq(Edge a, Edge b)
{
    return (
        (vec3_nearEq(a.a, b.a) && vec3_nearEq(a.b, b.b)) ||
        (vec3_nearEq(a.a, b.b) && vec3_nearEq(a.b, b.a)));
}

static inline int edge_hasVertex(Edge a, Vec3 p)
{
    return (vec3_nearEq(a.a, p) || vec3_nearEq(a.b, p));
}

inline static Vec3 edge_reflect(Vec3 ea, Vec3 eb, Vec3 point)
{
    float a = eb.y - ea.y;
    float b = ea.x - eb.x;
    float c = eb.x * ea.y - ea.x * eb.y;

    float footX = (b * (b * point.x - a * point.y) - a * c) / (a * a + b * b);
    float footY = (a * (a * point.y - b * point.x) - b * c) / (a * a + b * b);

    Vec3 out;
    out.z = 0;
    out.x = 2 * footX - point.x;
    out.y = 2 * footY - point.y;
    return out;
}
//

static inline Sphere triangle_circumsphereSqr(Triangle t)
{
    Sphere s;

    float ap = mat4_det((Mat4){1, 0, 0, 0,
                               0, t.a.x, t.b.x, t.c.x,
                               0, t.a.y, t.b.y, t.c.y,
                               0, 1, 1, 1});
    float a2 = vec3_sqrMag(t.a);
    float b2 = vec3_sqrMag(t.b);
    float c2 = vec3_sqrMag(t.c);

    float bx = -mat4_det((Mat4){1, 0, 0, 0,
                                0, a2, b2, c2,
                                0, t.a.y, t.b.y, t.c.y,
                                0, 1, 1, 1});

    float by = mat4_det((Mat4){1, 0, 0, 0,
                               0, a2, b2, c2,
                               0, t.a.x, t.b.x, t.c.x,
                               0, 1, 1, 1});

    float cp = -mat4_det((Mat4){1, 0, 0, 0,
                                0, a2, b2, c2,
                                0, t.a.x, t.b.x, t.c.x,
                                0, t.a.y, t.b.y, t.c.y});

    s.position = vec3(-bx / (2 * ap), -by / (2 * ap), 0);
    s.radius = ((bx * bx) + (by * by) - (4 * (ap * cp))) / (4 * (ap * ap));
    return s;
}

static inline int triangle_circumfere(Triangle t, Vec3 p)
{
    Sphere s = triangle_circumsphereSqr(t);
    return vec3_sqrMag(vec3_sub(s.position, p)) <= s.radius;
}

static inline int triangle_eq(Triangle a, Triangle b)
{
    return (vec3_eq(a.a, b.a) && vec3_eq(a.b, b.b) && vec3_eq(a.c, b.c)) ||
           vec3_eq(a.a, b.b) && vec3_eq(a.b, b.c) && vec3_eq(a.c, b.a) ||
           vec3_eq(a.a, b.c) && vec3_eq(a.b, b.a) && vec3_eq(a.c, b.b);
}

static inline int triangle_nearEq(Triangle a, Triangle b)
{
    return (vec3_nearEq(a.a, b.a) && vec3_nearEq(a.b, b.b) && vec3_nearEq(a.c, b.c)) ||
           vec3_nearEq(a.a, b.b) && vec3_nearEq(a.b, b.c) && vec3_nearEq(a.c, b.a) ||
           vec3_nearEq(a.a, b.c) && vec3_nearEq(a.b, b.a) && vec3_nearEq(a.c, b.b);
}

static inline int triangle_hasVertex(Triangle a, Vec3 p)
{
    return (vec3_nearEq(a.a, p) || vec3_nearEq(a.b, p) || vec3_nearEq(a.c, p));
}

static inline int triangle_hasEdge(Triangle a, Edge p)
{
    return (vec3_nearEq(a.a, p.a) && vec3_nearEq(a.b, p.b)) ||
           (vec3_nearEq(a.a, p.b) && vec3_nearEq(a.b, p.a)) ||
           (vec3_nearEq(a.b, p.a) && vec3_nearEq(a.c, p.b)) ||
           (vec3_nearEq(a.b, p.b) && vec3_nearEq(a.c, p.a)) ||
           (vec3_nearEq(a.a, p.a) && vec3_nearEq(a.c, p.b)) ||
           (vec3_nearEq(a.a, p.b) && vec3_nearEq(a.c, p.a));
}

static inline Triangle triangle_supra(Vec3 *vertices, int n, float offset)
{
    BBox b = bbox_calculate(vertices, n);
    b = bbox_expand(b, offset);
    float max = fmaxf(bbox_width(b), bbox_depth(b)) * 2;

    Vec3 d = vec3_norm(vec3_sub(b.min, b.max));

    Triangle t;
    t.a = b.min;
    t.b = b.min;
    t.c = b.min;
    t.b.y += max;
    t.c.x += max;

    return t;
}

static inline Vec3 triangle_norm(Triangle t)
{
    return vec3_cross(vec3_sub(t.b, t.a), vec3_sub(t.c, t.a));
}

static inline void triangle_edges(Triangle t, Edge edges[3])
{
    edges[0] = (Edge){t.a, t.b};
    edges[1] = (Edge){t.b, t.c};
    edges[2] = (Edge){t.c, t.a};
}

static inline Quad quad(Vec3 a, Vec3 b, Vec3 c, Vec3 d)
{
    Quad t;
    t.a = a;
    t.b = b;
    t.c = c;
    t.d = d;
    return t;
}
static inline Quad quad_rot(Quad q, Rot r)
{
    q.a = rot_rotate(r, q.a);
    q.b = rot_rotate(r, q.b);
    q.c = rot_rotate(r, q.c);
    q.d = rot_rotate(r, q.d);
    return q;
}
static inline Quad quad_offset(Quad q, Vec3 b)
{
    q.a = vec3_add(b, q.a);
    q.b = vec3_add(b, q.b);
    q.c = vec3_add(b, q.c);
    q.d = vec3_add(b, q.d);
    return q;
}

static inline Edge edge_offset(Edge q, Vec3 b)
{
    q.a = vec3_add(b, q.a);
    q.b = vec3_add(b, q.b);
    return q;
}

static inline Triangle triangle_offset(Triangle q, Vec3 b)
{
    q.a = vec3_add(b, q.a);
    q.b = vec3_add(b, q.b);
    q.c = vec3_add(b, q.c);
    return q;
}
static inline Triangle triangle(Vec3 a, Vec3 b, Vec3 c)
{
    Triangle t;
    t.a = a;
    t.b = b;
    t.c = c;
    return t;
}

static inline Triangle triangle_rot(Triangle t, Rot r)
{
    t.a = rot_rotate(r, t.a);
    t.b = rot_rotate(r, t.b);
    t.c = rot_rotate(r, t.c);
    return t;
}

static inline Sphere sphere(Vec3 position, float radius)
{
    Sphere s;
    s.position = position;
    s.radius = radius;
    return s;
}

static inline BBox calculate_bbox(Vec3 vertices[], int n, float offset)
{
    if (n == 0)
        return bbox_empty;

    Vec3 min = vertices[0];
    Vec3 max = vertices[0];

    for (int i = 1; i < n; i++)
    {
        min = vec3_min(min, vertices[i]);
        max = vec3_max(max, vertices[i]);
    }

    min = vec3_add(min, vec3f(-offset));
    max = vec3_add(max, vec3f(offset));

    return (BBox){min, max};
}

static inline Triangle calculate_supra(BBox b)
{
    float max = fmaxf(bbox_width(b), bbox_depth(b)) * 2;
    return (Triangle){b.min, vec3_add(b.min, vec3_mulf(vec3_right, max)), vec3_add(b.min, vec3_mulf(vec3_forward, max))};
}

#endif