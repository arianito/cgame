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
    uint64_t tag;

    MeshId mesh;
    Material material;
} Sprite;

typedef struct {
    Sprite* it;
    int index;
} SpriteItter;

make_fastvec_directives(Sprite, Sprite);

void sprite_init();

SpriteId sprite_create(const char *model, const char *texture);
Sprite *sprite_get(SpriteId id);
void sprite_crop(SpriteId id, Rect r);
void sprite_crop_pixelart(SpriteId id, Vec2 idx, Vec2 dim);
void sprite_crop_pixelart_id(SpriteId id, uint32_t hexCode);
void sprite_delete(SpriteId id);

void sprite_anim(SpriteId id, uint32_t ids[], uint32_t n, float fps, float t0);

void sprite_clear();
void sprite_render();
void sprite_destroy();

SpriteItter sprite_begin();
bool sprite_eof(SpriteItter* self);
void sprite_next(SpriteItter* self);

#define sprite_for(it) for (SpriteItter it = sprite_begin(); !sprite_eof(&it); sprite_next(&it))

#endif