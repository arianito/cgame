#ifndef cgame_COLOR_H
#define cgame_COLOR_H

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
Color color_lch(Color rgb);
Color color_from_lchf(float l, float c, float h, float a);
Color color_from_lch(Color lch);
Color color_lerp_lch(Color lchA, Color lchB, float t);
//
Color color_hslf(float r, float g, float b, float a);
Color color_hsl(Color rgb);
Color color_from_hslf(float h, float s, float l, float a);
Color color_from_hsl(Color hsl);
//
Color color_alpha(Color c, float a);
Color color_lerp(Color a, Color b, float d);
Color color_rand();
Color color_darken(Color rgba, float p);
Color color_lighten(Color rgba, float p);

#endif