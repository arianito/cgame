#include "atlas.h"

#include <pthread.h>
#include "mem/alloc.h"

#include "file.h"
#include "stb_image.h"

#include "glad.h"

#define MODE_REST 0
#define MODE_CREATE 1
#define MODE_DELETE 2
#define MODE_CLEAR 3

typedef struct
{
    Fastmap_StrInt *indices;
    Fastvec_Tex *textures;

    pthread_cond_t cond;
    pthread_mutex_t lock;

    int mode;
    uint8_t *data;
    Texture temp;
} AtlasContext;

static AtlasContext *self;

void atlas_init()
{
    self = (AtlasContext *)arena_alloc(alloc->global, sizeof(AtlasContext), sizeof(size_t));
    self->indices = fastmap_StrInt_init();
    self->textures = fastvec_Tex_init(2);

    self->mode = MODE_REST;
    pthread_cond_init(&self->cond, NULL);
    pthread_mutex_init(&self->lock, NULL);
}

void atlas_update()
{
    if (self->mode != MODE_REST)
    {
        Texture *t = &self->temp;
        switch (self->mode)
        {
        case MODE_CREATE:
            glGenTextures(1, &t->gid);
            glBindTexture(GL_TEXTURE_2D, t->gid);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)t->size.x, (GLsizei)t->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, self->data);
            glGenerateMipmap(GL_TEXTURE_2D);
            break;
        case MODE_DELETE:
            glDeleteTextures(1, &t->gid);
            break;
        case MODE_CLEAR:
            for (int i = 0; i < self->textures->length; i++)
            {
                glDeleteTextures(1, &self->textures->vector[i].gid);
            }
            fastvec_Tex_clear(self->textures);
            fastmap_StrInt_clear(self->indices);
            break;
        }
        pthread_cond_signal(&self->cond);
        self->mode = MODE_REST;
    }
}

Texture *atlas_load(const char *name, const char *p)
{
    Texture *tex = &self->temp;
    tex->name = name;

    int width, height;
    char *path = resolve_stack(p);
    uint8_t *data = stbi_load(path, &width, &height, &tex->channels, 0);
    stack_free(alloc->stack, (void *)path);
    if (data == NULL)
        return NULL;

    tex->size.x = (float)width;
    tex->size.y = (float)height;
    tex->ratio = tex->size.y / tex->size.x;
    pthread_mutex_lock(&self->lock);
    self->data = data;
    self->mode = MODE_CREATE;
    pthread_cond_wait(&self->cond, &self->lock);
    pthread_mutex_unlock(&self->lock);
    stbi_image_free(data);
    // map
    self->temp.id = self->indices->length;
    fastvec_Tex_push(self->textures, self->temp);
    FastmapNode_StrInt *node = fastmap_StrInt_put(self->indices, name);
    node->value = self->temp.id;

    return &self->textures->vector[self->temp.id];
}

Texture *atlas_get_byname(const char *name)
{
    FastmapNode_StrInt *node = fastmap_StrInt_get(self->indices, name);
    if (node == NULL)
        return NULL;

    return &self->textures->vector[node->value];
}

Texture *atlas_get(int id)
{
    return &self->textures->vector[id];
}

bool atlas_has(int id)
{
    return id >= 0 && id < self->textures->length;
}

void atlas_clear() {
    self->mode = MODE_CLEAR;
}

void atlas_destroy()
{
    pthread_cond_signal(&self->cond);
    for (int i = 0; i < self->textures->length; i++)
    {
        glDeleteTextures(1, &self->textures->vector[i].gid);
    }
    pthread_cond_destroy(&self->cond);
    pthread_mutex_destroy(&self->lock);

    fastmap_StrInt_destroy(self->indices);
    fastvec_Tex_destroy(self->textures);
}
