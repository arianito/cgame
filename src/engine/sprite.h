#ifndef cgame_SPRITE_H
#define cgame_SPRITE_H

#include "engine/mathf.h"

typedef struct {
    int id;
    Vec3 position;
    Rot rotation;
    
    Vec2 scale;
    Rect crop;

    int texture_id;
} Sprite;


void sprite_init();
void sprite_render();
void sprite_destroy();

#endif