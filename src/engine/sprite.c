#include "sprite.h"

#include <string.h>
#include "mem/alloc.h"
#include "shader.h"
#include "camera.h"

typedef struct
{
    Shader shader;
    GLuint vaoIds[1];
    GLuint vboIds[1];
    GLuint eboIds[1];
} SpriteContext;

static SpriteContext *self = NULL;

void sprite_init()
{
    if (self != NULL)
    {
        printf("already have an instance of sprite running.");
        exit(0);
    }

    self = (SpriteContext *)xxmalloc(sizeof(SpriteContext));
    memset(self, 0, sizeof(SpriteContext));

    self->shader = shader_load("shaders/sprite.vs", "shaders/sprite.fs");

    glGenVertexArrays(1, self->vaoIds);
    glGenBuffers(1, self->vboIds);
    glGenBuffers(1, self->eboIds);

    float w = 100;
    float h = 100;
    VertexData vertices[] = {
        {vec3(0, -w, h), vec2(0, 0)},
        {vec3(0, w, h), vec2(1, 0)},
        {vec3(0, w, -h), vec2(1, 1)},
        {vec3(0, -w, -h), vec2(0, 1)},
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

void sprite_render()
{
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    shader_begin(self->shader);

    shader_mat4(self->shader, "projection", &camera->projection);
    shader_mat4(self->shader, "view", &camera->view);
    shader_mat4(self->shader, "world", &mat4_identity);
    shader_vec4(self->shader, "color", &color_red);

    glBindVertexArray(self->vaoIds[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    shader_end();
}

void sprite_destroy()
{
    glDeleteVertexArrays(1, self->vaoIds);
    glDeleteBuffers(1, self->vboIds);
    glDeleteBuffers(1, self->eboIds);
    shader_destroy(self->shader);
    xxfree(self);
    self = NULL;
}