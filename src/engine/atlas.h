#ifndef cgame_ATLAS_H
#define cgame_ATLAS_H

#include "engine/mathf.h"

typedef struct
{
    int id;
    float width;
    float height;
} Texture;

typedef struct
{
    int length;
    Texture textures[64];
} Atlas;

void atlas_init();

void atlas_load(const char *path);

void atlas_destroy();

#endif