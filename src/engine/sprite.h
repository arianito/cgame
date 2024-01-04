#ifndef cgame_SPRITE_H
#define cgame_SPRITE_H

#include "engine/mathf.h"
#include "adt/fastvec.h"
#include "atlas.h"

#define SP_FLAG_TWO_SIDED 1 << 1
#define SP_FLAG_FLIPPED 1 << 2
#define SP_FLAG_PIXELART 1 << 3

typedef struct
{
    int id;
    Vec3 position;
    Rot rotation;

    Vec2 scale;
    Vec2 origin;
    Rect crop;
    float basis;

    uint32_t flags;

    int texture;
} Sprite;

make_fastvec_directives(Sprite, Sprite);

void sprite_init();

void sprite_crop(int id, Rect r);
int sprite_create(const char *name);
void sprite_delete(int id);
Sprite * sprite_get(int id);

void sprite_clear();
void sprite_render();
void sprite_destroy();

#endif