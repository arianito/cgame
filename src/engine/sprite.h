#ifndef cgame_SPRITE_H
#define cgame_SPRITE_H

#include "atlas.h"
#include "math/rect.h"
#include "math/vec3.h"
#include "math/rot.h"
#include "adt/fastvec.h"

#define MAT_FLAG_TWO_SIDED 1 << 1
#define MAT_FLAG_FLIPPED 1 << 2
#define MAT_FLAG_PIXELART 1 << 3
#define MAT_FLAG_ALPHAMASK 1 << 4

typedef uint32_t SpriteId;

typedef struct
{
    int texture;
    float mask_threshold;
    Rect cropped_area;
    uint32_t flags;
} Material;

typedef struct
{
    SpriteId id;
    Vec3 position;
    Rot rotation;

    Vec2 scale;
    Vec2 origin;
    float ratio;
    uint32_t flags;

    Material material;
} Sprite;

make_fastvec_directives(Sprite, Sprite);

void sprite_init();

SpriteId sprite_create(const char *name);
Sprite *sprite_get(SpriteId id);
void sprite_crop(SpriteId id, Rect r);
void sprite_crop_pixelart(SpriteId id, Vec2 idx, Vec2 dim);
void sprite_crop_pixelart_id(SpriteId id, uint32_t hexCode);
void sprite_delete(SpriteId id);

void sprite_clear();
void sprite_render();
void sprite_destroy();

#endif