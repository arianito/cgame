#include "sprite.h"

#include <string.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad.h"

#include "mem/alloc.h"
#include "shader.h"
#include "camera.h"

typedef struct
{
    Vec3 position;
    Vec2 coord;
} VertexData;

typedef struct
{
    Shader shader;
    GLuint vaoIds[1];
    GLuint vboIds[1];
    GLuint eboIds[1];

    Fastvec_Sprite *sprites;
} SpriteContext;

static SpriteContext *self = NULL;

int sprite_create(const char *name)
{
    Texture *tex = atlas_get_byname(name);
    if (tex == NULL)
        return -1;

    Sprite sp;
    sp.id = self->sprites->length;
    sp.position = vec3_zero;
    sp.rotation = rot_zero;
    sp.basis = 100;
    sp.origin = vec2(0.5, 0.5);
    sp.scale = vec2(1, 1);

    sp.material.texture = tex->id;
    sp.material.mask_threshold = 0.5;
    sp.material.flags = MAT_FLAG_TWO_SIDED | MAT_FLAG_PIXELART | MAT_FLAG_ALPHAMASK;
    sp.material.cropped_area = rect(0, 0, tex->size.x, tex->size.y);

    fastvec_Sprite_push(self->sprites, sp);
    return sp.id;
}

void sprite_delete(int id)
{
    fastvec_Sprite_remove(self->sprites, id);
}

Sprite *sprite_get(int id)
{
    return &self->sprites->vector[id];
}
void sprite_crop(int id, Rect r)
{
    Sprite *sp = &self->sprites->vector[id];
    sp->material.cropped_area = r;
}
void sprite_init()
{
    self = (SpriteContext *)xxmalloc(sizeof(SpriteContext));
    memset(self, 0, sizeof(SpriteContext));
    self->sprites = fastvec_Sprite_init(8);
    self->shader = shader_load("shaders/sprite.vs", "shaders/sprite.fs");

    // quad
    glGenBuffers(1, self->vboIds);
    glGenBuffers(1, self->eboIds);
    glGenVertexArrays(1, self->vaoIds);

    float w = 0.5;
    float h = 0.5;
    VertexData vertices[] = {
        {vec3(0, w, h), vec2(0, 0)},
        {vec3(0, -w, h), vec2(1, 0)},
        {vec3(0, -w, -h), vec2(1, 1)},
        {vec3(0, w, -h), vec2(0, 1)},
    };
    unsigned int indices[] = {0, 1, 3, 1, 2, 3};
    glBindVertexArray(self->vaoIds[0]);
    glBindBuffer(GL_ARRAY_BUFFER, self->vboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->eboIds[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void *)offsetof(VertexData, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void *)offsetof(VertexData, coord));

}

void draw_sprites(Shader sh)
{
    shader_texture(sh, "texture1", 0);
    shader_mat4(self->shader, "view_projection", &camera->view_projection);
    for (int i = 0; i < self->sprites->length; i++)
    {
        Sprite *it = &self->sprites->vector[i];
        if (!atlas_has(it->material.texture))
            continue;

        Texture *tex = atlas_get(it->material.texture);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex->gid);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Mat4 m = rot_matrix(it->rotation, it->position);
        shader_mat4(sh, "world", &m);
        shader_vec2(sh, "origin", &it->origin);
        shader_float(sh, "basis", it->basis);
        shader_float(sh, "threshold", it->material.mask_threshold);
        shader_vec4(sh, "crop", &it->material.cropped_area);
        shader_vec2(sh, "tex_size", &tex->size);
        shader_vec2(sh, "scale", &it->scale);
        shader_int(sh, "pixelart", (it->material.flags & MAT_FLAG_PIXELART) == MAT_FLAG_PIXELART);

        if (!(it->material.flags & MAT_FLAG_TWO_SIDED))
        {
            glEnable(GL_CULL_FACE);
            glCullFace((it->material.flags & MAT_FLAG_FLIPPED) ? GL_BACK : GL_FRONT);
            glFrontFace(GL_CW);
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }
        glBindVertexArray(self->vaoIds[0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}
void sprite_render()
{
    if (self->sprites->length == 0)
        return;

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    shader_begin(self->shader);
    draw_sprites(self->shader);
    shader_end();
}

void sprite_clear()
{
    fastvec_Sprite_clear(self->sprites);
}

void sprite_destroy()
{
    fastvec_Sprite_destroy(self->sprites);
    glDeleteVertexArrays(1, self->vaoIds);
    glDeleteBuffers(1, self->vboIds);
    glDeleteBuffers(1, self->eboIds);
    shader_destroy(self->shader);
    xxfree(self);
    self = NULL;
}