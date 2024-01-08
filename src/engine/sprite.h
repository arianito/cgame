#ifndef cgame_SPRITE_H
#define cgame_SPRITE_H

#include "sprite_base.h"
#include "atlas.h"
#include "math/rect.h"
#include "math/vec3.h"
#include "math/rot.h"
#include "adt/fastvec.h"
#include "mesh.h"

typedef struct
{
    SpriteId id;
    Vec3 position;
    Rot rotation;
    Vec3 scale;

    MeshId mesh;
    Material material;
} Sprite;

make_fastvec_directives(Sprite, Sprite);

void sprite_init();

SpriteId sprite_create(const char *model, const char* texture);
Sprite *sprite_get(SpriteId id);
void sprite_crop(SpriteId id, Rect r);
void sprite_crop_pixelart(SpriteId id, Vec2 idx, Vec2 dim);
void sprite_crop_pixelart_id(SpriteId id, uint32_t hexCode);
void sprite_delete(SpriteId id);

void sprite_clear();
void sprite_render();
void sprite_destroy();

#endif