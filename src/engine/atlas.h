#ifndef cgame_ATLAS_H
#define cgame_ATLAS_H

#include "glad.h"
#include "engine/mathf.h"
#include "adt/murmur.h"
#include "adt/fastmap.h"
#include "adt/fastvec.h"
#include <string.h>

inline static bool __compare_string(char *a, char *b)
{
    return strcmp(a, b) == 0;
}

inline static uint64_t __hash_string(char *key, uint64_t seed)
{
    return murmurhash(key, strlen(key), seed);
}

make_fastmap_directives(StrInt, char *, int, __compare_string, __hash_string);

typedef struct
{
    int _id;
    GLuint gid;
    char *name;
    int channels;
    Vec2 size;
    float ratio;
} Texture;

make_fastvec_directives(Tex, Texture);

void atlas_init();

Texture *atlas_load(const char *name, const char *p);

Texture *atlas_get_byname(const char *name);

Texture *atlas_get(int id);

bool atlas_has(int id);

void atlas_update();

void atlas_clear();

void atlas_unload(const char *name);

void atlas_destroy();

#endif