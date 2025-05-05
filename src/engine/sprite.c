#include "sprite.h"

#include <string.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad.h"

#include "mem/alloc.h"
#include "shader.h"
#include "camera.h"
#include "math/rect.h"
#include <stdio.h>

typedef struct
{
    Vec3 position;
    Vec3 normal;
    Vec2 coord;
} VertexData;

typedef struct
{
    Shader shader;
    Fastvec_Sprite *sprites;
} SpriteContext;

static SpriteContext *self = NULL;

SpriteId sprite_create(const char *model, const char *texture)
{
    Mesh *mesh = mesh_get_byname(model);

    if (mesh == NULL)
        return -1;

    Texture *tex = atlas_get_byname(texture);
    if (tex == NULL)
        return -1;
    Sprite sp;
    sp.id = self->sprites->length;
    sp.position = vec3_zero;
    sp.rotation = rot_zero;
    sp.scale = vec3(1, 1, 1);
    sp.mesh = mesh->id;
    sp.tag = 0;

    sp.material.texture = tex->id;
    sp.material.mask_threshold = 0.5;
    sp.material.flags = MAT_FLAG_TWO_SIDED | MAT_FLAG_PIXELART | MAT_FLAG_ALPHAMASK;
    sp.material.cropped_area = rect(0, 0, 0, 0);
    fastvec_Sprite_push(self->sprites, sp);
    return sp.id;
}

void sprite_delete(SpriteId id)
{
    fastvec_Sprite_remove(self->sprites, id);
}

Sprite *sprite_get(SpriteId id)
{
    return &self->sprites->vector[id];
}
void sprite_crop(SpriteId id, Rect r)
{
    Sprite *sp = &self->sprites->vector[id];
    sp->material.cropped_area = r;
}
void sprite_crop_pixelart(SpriteId id, Vec2 idx, Vec2 dim)
{
    Sprite *sp = &self->sprites->vector[id];
    sp->material.cropped_area = rectv(vec2_mulv(idx, dim), dim);
}
void sprite_crop_pixelart_id(SpriteId id, uint32_t hexCode)
{
    Sprite *sp = &self->sprites->vector[id];

    float x = (float)((hexCode >> 24) & 0xFF);
    float y = (float)((hexCode >> 16) & 0xFF);
    float w = (float)((hexCode >> 8) & 0xFF);
    float h = (float)(hexCode & 0xFF);

    sp->material.cropped_area = rect(x * w, y * h, w, h);
}
void sprite_init()
{
    self = (SpriteContext *)xxmalloc(sizeof(SpriteContext));
    memset(self, 0, sizeof(SpriteContext));
    self->sprites = fastvec_Sprite_init(8);
    self->shader = shader_load("shaders/sprite.vs", "shaders/sprite.fs");
}

void sprite_render()
{
    if (self->sprites->length == 0)
        return;

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    Shader sh = self->shader;
    shader_begin(sh);
    shader_texture(sh, "texture1", 0);
    shader_mat4(self->shader, "view_projection", &camera->view_projection);
    for (int i = 0; i < self->sprites->length; i++)
    {
        Sprite *it = &self->sprites->vector[i];
        if (!atlas_has(it->material.texture) || !mesh_has(it->mesh))
            continue;

        Texture *tex = atlas_get(it->material.texture);
        Mesh *mesh = mesh_get(it->mesh);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex->gid);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Mat4 m = mat4_mul(mat4_scale(it->scale), rot_matrix(it->rotation, it->position));
        shader_mat4(sh, "world", &m);
        shader_float(sh, "threshold", it->material.mask_threshold);
        shader_vec4(sh, "crop", &it->material.cropped_area);
        shader_vec2(sh, "tex_size", &tex->size);
        shader_int(sh, "pixelart", (it->material.flags & MAT_FLAG_PIXELART) == MAT_FLAG_PIXELART);
        if (!(it->material.flags & MAT_FLAG_TWO_SIDED))
        {
            glEnable(GL_CULL_FACE);
            glCullFace((it->material.flags & MAT_FLAG_FLIPPED) ? GL_BACK : GL_FRONT);
            glFrontFace(GL_CCW);
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }
        glBindVertexArray(mesh->vao);
        glDrawElements(GL_TRIANGLES, mesh->length, GL_UNSIGNED_INT, NULL);
    }

    shader_end();
}

void sprite_clear()
{
    fastvec_Sprite_clear(self->sprites);
}

void sprite_destroy()
{
    fastvec_Sprite_destroy(self->sprites);
    shader_destroy(self->shader);
    xxfree(self, sizeof(SpriteContext));
    self = NULL;
}

void sprite_anim(SpriteId id, uint32_t ids[], uint32_t n, float fps, float t0)
{

    uint32_t t = (int)((gtime->elapsed - t0) * fps);
    int i = t % n;
    sprite_crop_pixelart_id(id, ids[i]);
}

SpriteItter sprite_begin()
{
    return (SpriteItter){&self->sprites->vector[0], 0};
}

bool sprite_eof(SpriteItter *it)
{
    return it->index == self->sprites->length;
}

void sprite_next(SpriteItter *it)
{
    it->it = &self->sprites->vector[++it->index];
}