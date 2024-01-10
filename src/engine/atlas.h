#ifndef cgame_ATLAS_H
#define cgame_ATLAS_H


#include "math/vec2.h"
#include "adt/murmur.h"
#include "adt/fastmap.h"
#include "adt/fastvec.h"
#include "adt/common.h"

typedef int32_t TextureId;

typedef struct
{
    TextureId id;
    uint32_t gid;
    const char *name;
    int channels;
    Vec2 size;
    float ratio;
} Texture;


void atlas_init();

Texture *atlas_load(const char *name, const char *p);

Texture *atlas_get_byname(const char *name);

Texture *atlas_get(TextureId id);

bool atlas_has(TextureId id);

void atlas_clear();

void atlas_destroy();

#endif