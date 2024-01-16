#ifndef cgame_COLOR_H
#define cgame_COLOR_H

#include "scalar.h"
#include "rand.h"

typedef struct
{
    float p0;
    float p1;
    float p2;
    float alpha;
} Color;


#define color(r, g, b, a) ((Color){r, g, b, a})

static const Color color_black = {0, 0, 0, 1};
static const Color color_white = {1, 1, 1, 1};
static const Color color_red = {0.737f, 0.278f, 0.286f, 1};
static const Color color_green = {0.655f, 0.788f, 0.341f, 1};
static const Color color_blue = {0.129f, 0.619f, 0.737f, 1};
static const Color color_yellow = {1.0f, 0.718f, 0.012f, 1};
static const Color color_orange = {0.984f, 0.521f, 0, 1};
static const Color color_gray = {0.5f, 0.5f, 0.5f, 1};


Color color_lchf(float r, float g, float b, float a);
Color color_from_lchf(float l, float c, float h, float a);
Color color_hslf(float r, float g, float b, float a);
Color color_from_hslf(float h, float s, float l, float a);


inline static uint32_t color_hex(Color rgba) {
    uint8_t r = (uint8_t)(rgba.p0 * 255);
    uint8_t g = (uint8_t)(rgba.p1 * 255);
    uint8_t b = (uint8_t)(rgba.p2 * 255);
    uint8_t a = (uint8_t)(rgba.alpha * 255);
    return (a << 24) | (b << 16) | (g << 8) | r;
}

inline static Color color_lch(Color rgb)
{
    return color_lchf(rgb.p0, rgb.p1, rgb.p2, rgb.alpha);
}

inline static Color color_from_lch(Color lch)
{
    return color_from_lchf(lch.p0, lch.p1, lch.p2, lch.alpha);
}

inline static Color color_lerp_lch(Color a, Color b, float t) {
    a.p0 = a.p0 + (b.p0 - a.p0) * t;
    a.p1 = a.p1 + (b.p1 - a.p1) * t;
    a.p2 = a.p2 + (b.p2 - a.p2) * t;
    a.alpha = a.alpha + (b.alpha - a.alpha) * t;
    a.p2 = clamp_axisf(a.p2);
    return a;
}

inline static Color color_hsl(Color rgb)
{
    return color_hslf(rgb.p0, rgb.p1, rgb.p2, rgb.alpha);
}

inline static Color color_from_hsl(Color hsl)
{
    return color_from_hslf(hsl.p0, hsl.p1, hsl.p2, hsl.alpha);
}

inline static Color color_alpha(Color c, float a)
{
    c.alpha = a;
    return c;
}

inline static Color color_lerp(Color a, Color b, float t)
{
    a.alpha = a.alpha + (b.alpha - a.alpha) * t;
    a.p0 = a.p0 + (b.p0 - a.p0) * t;
    a.p1 = a.p1 + (b.p1 - a.p1) * t;
    a.p2 = a.p2 + (b.p2 - a.p2) * t;
    return a;
}

inline static Color color_rand()
{
    return color(randf(), randf(), randf(), 1);
}

inline static Color color_darken(Color rgba, float p)
{
    Color hsl = color_hsl(rgba);
    hsl.p2 = clampf(hsl.p2 * (1 - p), 0.0f, 1.0f);
    return color_from_hsl(hsl);
}

inline static Color color_lighten(Color rgba, float p)
{
    Color hsl = color_hsl(rgba);
    hsl.p2 = clampf(hsl.p2 * (1 + p), 0.0f, 1.0f);
    return color_from_hsl(hsl);
}


#endif