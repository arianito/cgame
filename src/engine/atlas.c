#include "atlas.h"

#include <pthread.h>
#include "mem/alloc.h"

#include "file.h"
#include "stb_image.h"

#include "glad.h"

typedef struct
{
    Fastmap_StrTexId *indices;
    Fastvec_Tex *textures;
} AtlasContext;

static AtlasContext *self;

void atlas_init()
{
    self = (AtlasContext *)xxarena(sizeof(AtlasContext));
    self->indices = fastmap_StrTexId_init();
    self->textures = fastvec_Tex_init(2);
}

Texture *atlas_load(const char *name, const char *p)
{
    FastmapNode_StrTexId *node = fastmap_StrTexId_get(self->indices, name);
    if (node != NULL)
        return &self->textures->vector[node->value];
    Texture tex;
    tex.name = name;

    int width, height;
    StrView path = resolve_stack(p);
    uint8_t *data = stbi_load(path.string, &width, &height, &tex.channels, 0);
    xxfreestack(path.string);
    if (data == NULL)
        return NULL;

    tex.size.x = (float)width;
    tex.size.y = (float)height;
    tex.ratio = vec2_ratio(tex.size);
    GLenum type = 0;
    switch (tex.channels)
    {
    case 3:
        type = GL_RGB;
        break;
    case 4:
        type = GL_RGBA;
        break;
    default:
        stbi_image_free(data);
        return NULL;
    }

    glGenTextures(1, &tex.gid);
    glBindTexture(GL_TEXTURE_2D, tex.gid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)tex.size.x, (GLsizei)tex.size.y, 0, type, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    //
    stbi_image_free(data);
    // map
    tex.id = self->indices->length;
    fastvec_Tex_push(self->textures, tex);
    node = fastmap_StrTexId_put(self->indices, name);
    node->value = tex.id;
    return &self->textures->vector[tex.id];
}

Texture *atlas_get_byname(const char *name)
{
    FastmapNode_StrTexId *node = fastmap_StrTexId_get(self->indices, name);
    if (node == NULL)
        return NULL;

    return &self->textures->vector[node->value];
}

Texture *atlas_get(TextureId id)
{
    return &self->textures->vector[id];
}

bool atlas_has(TextureId id)
{
    return id >= 0 && id < self->textures->length;
}

void atlas_clear()
{
    for (int i = 0; i < self->textures->length; i++)
    {
        glDeleteTextures(1, &self->textures->vector[i].gid);
    }
    fastvec_Tex_clear(self->textures);
    fastmap_StrTexId_clear(self->indices);
}

void atlas_destroy()
{
    for (int i = 0; i < self->textures->length; i++)
    {
        glDeleteTextures(1, &self->textures->vector[i].gid);
    }
    fastmap_StrTexId_destroy(self->indices);
    fastvec_Tex_destroy(self->textures);
}
