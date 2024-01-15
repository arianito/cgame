#include "color.h"
#include "vec3.h"
#include "vec4.h"
#include "defs.h"

//

Color color_lchf(float r, float g, float b, float a)
{
    r = ((r > 0.04045) ? powerf((r + 0.055) / 1.055, 2.4) : r / 12.92);
    g = ((g > 0.04045) ? powerf((g + 0.055) / 1.055, 2.4) : g / 12.92);
    b = ((b > 0.04045) ? powerf((b + 0.055) / 1.055, 2.4) : b / 12.92);

    float x = (r * 0.4124564 + g * 0.3575761 + b * 0.1804375) * (100.0 / 95.047);
    float y = (r * 0.2126729 + g * 0.7151522 + b * 0.0721750);
    float z = (r * 0.0193339 + g * 0.1191920 + b * 0.9503041) * (100.0 / 108.883);

    x = (x > 0.008856) ? powerf(x, 1.0 / 3.0) : (903.3 * x + 16.0) / 116.0;
    y = (y > 0.008856) ? powerf(y, 1.0 / 3.0) : (903.3 * y + 16.0) / 116.0;
    z = (z > 0.008856) ? powerf(z, 1.0 / 3.0) : (903.3 * z + 16.0) / 116.0;

    float l2 = (116 * y) - 16;
    float a2 = 500 * (x - y);
    float b2 = 200 * (y - z);

    return color(l2, sqrf(a2 * a2 + b2 * b2), clamp_axisf(atan2df(b2, a2)), a);
}


Color color_from_lchf(float l, float c, float h, float alpha)
{
    float a2 = c * cosdf(h);
    float b2 = c * sindf(h);

    float x, y, z, y2;

    if (l <= 8)
    {
        y = (l * 100) / 903.3;
        y2 = (7.787 * (y / 100)) + (16 / 116);
    }
    else
    {
        y = 100 * powerf((l + 16) / 116, 3);
        y2 = powerf(y / 100, 1.0 / 3.0);
    }

    x = (a2 / 500.0 + y2 - 16.0 / 116) <= 0.008856
            ? (95.047 * ((a2 / 500) + y2 - (16 / 116))) / 7.787
            : 95.047 * powerf((a2 / 500) + y2, 3);

    z = (b2 / 200.0 + y2 - 16.0 / 116) <= 0.008859
            ? (108.883 * (y2 - (b2 / 200) - (16 / 116))) / 7.787
            : 108.883 * powerf(y2 - (b2 / 200), 3);

    x /= 100;
    y /= 100;
    z /= 100;

    float r = (x * 3.2406) + (y * -1.5372) + (z * -0.4986);
    float g = (x * -0.9689) + (y * 1.8758) + (z * 0.0415);
    float b = (x * 0.0557) + (y * -0.2040) + (z * 1.0570);

    r = (r > 0.0031308) ? ((1.055 * powerf(r, 1.0 / 2.4)) - 0.055) : r * 12.92;
    g = (g > 0.0031308) ? ((1.055 * powerf(g, 1.0 / 2.4)) - 0.055) : g * 12.92;
    b = (b > 0.0031308) ? ((1.055 * powerf(b, 1.0 / 2.4)) - 0.055) : b * 12.92;
    return color(
        clamp01f(r),
        clamp01f(g),
        clamp01f(b),
        alpha);
}

Color color_hslf(float r, float g, float b, float a)
{
    float max = maxf(r, maxf(g, b));
    float min = minf(r, minf(g, b));

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
    return color(h, s, l, a);
}


Color color_from_hslf(float h, float s, float l, float a)
{
    float C = (1 - absf(2 * l - 1)) * s;
    float X = C * (1 - absf(moduluf(h / 60.0f, 2) - 1));
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
    return color(R+m, G+m, B+m, a);
}
