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

typedef struct
{
    int texture;
    float mask_threshold;
    Rect cropped_area;
    uint32_t flags;
} Material;

typedef struct
{
    int id;
    Vec3 position;
    Rot rotation;

    Vec2 scale;
    Vec2 origin;
    float basis;
    uint32_t flags;

    Material material;
} Sprite;

make_fastvec_directives(Sprite, Sprite);

void sprite_init();

void sprite_crop(int id, Rect r);
int sprite_create(const char *name);
void sprite_delete(int id);
Sprite *sprite_get(int id);

void sprite_clear();
void sprite_render();
void sprite_destroy();

#endif