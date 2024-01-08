#pragma once

#include <stdint.h>
#include "math/rect.h"
#include "atlas.h"

typedef uint32_t SpriteId;

#define MAT_FLAG_TWO_SIDED 1 << 1
#define MAT_FLAG_FLIPPED 1 << 2
#define MAT_FLAG_PIXELART 1 << 3
#define MAT_FLAG_ALPHAMASK 1 << 4

typedef struct
{
    TextureId texture;
    float mask_threshold;
    Rect cropped_area;
    uint32_t flags;
} Material;