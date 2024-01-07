
#include "debug.h"

#include <string.h>
#include <stdarg.h>

#include "mem/alloc.h"
#include "mem/defs.h"

#include "game.h"
#include "shader.h"
#include "camera.h"
#include "file.h"
#include "glad.h"
#include "stb_image.h"

typedef struct
{
    Vec3 position;
    Vec2 coord;
} VertexData;

enum
{
    max_space = 1 * MEGABYTES,
    max_elements = 4096,
};

typedef struct
{

    Vec3 position;
    Rot rotation;
    float scale;

    Vec2 origin;
    Color color;
    const char *text;

} Text3DData;

typedef struct
{
    Vec2 position;
    Vec2 origin;
    float scale;
    Color color;
    const char *text;

} Text2DData;

typedef struct
{
    Shader shader;
    GLuint vaoIds[2];
    GLuint vboIds[2];
    GLuint eboIds[2];

    Text2DData data2d[max_elements];
    Text3DData data3d[max_elements];

    int count2d;
    int count3d;

    GLuint fontTexture[1];

    Vec2 bound2d;
    Vec2 bound3d;
    float scale;
    Rot rotation;
    Vec2 origin;
    Color color;

    char enabled;

    ArenaMemory *arena;

} DrawData;

static DrawData *debugData;

void debug_init()
{
    debugData = (DrawData *)arena_alloc(alloc->global, sizeof(DrawData));
    memset(debugData, 0, sizeof(DrawData));

    debugData->arena = arena_create(arena_alloc(alloc->global, max_space), max_space);

    debugData->enabled = 1;
    debugData->origin = vec2_zero;
    debugData->color = color_white;
    debugData->scale = 1.0f;
    debugData->rotation = rot_zero;
    debugData->shader = shader_load("shaders/debug.vs", "shaders/debug.fs");

    // vao
    glGenVertexArrays(2, debugData->vaoIds);
    glGenBuffers(2, debugData->vboIds);
    glGenBuffers(2, debugData->eboIds);

    {
        float w = 8.0f;
        float h = 24.0f;
        debugData->bound2d = vec2(w, h);
        float offset = (h - w) / (h * 2.0f);
        VertexData vertices[] = {
            {vec3(0, 0, 0), vec2(offset, 0)},
            {vec3(w, 0, 0), vec2(1 - offset, 0)},
            {vec3(w, h, 0), vec2(1 - offset, 1)},
            {vec3(0, h, 0), vec2(offset, 1)},
        };
        unsigned int indices[] = {0, 1, 3, 1, 2, 3};

        glBindVertexArray(debugData->vaoIds[1]);
        glBindBuffer(GL_ARRAY_BUFFER, debugData->vboIds[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debugData->eboIds[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void *)offsetof(VertexData, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void *)offsetof(VertexData, coord));
    }

    {
        float w = 2.5f;
        float h = 8.0f;
        debugData->bound3d = vec2(w, h);
        float offset = (h - w) / (h * 2.0f);
        VertexData vertices[] = {
            {vec3(0, 0, 0), vec2(offset, 0)},
            {vec3(0, -w, 0), vec2(1 - offset, 0)},
            {vec3(0, -w, -h), vec2(1 - offset, 1)},
            {vec3(0, 0, -h), vec2(offset, 1)},
        };
        unsigned int indices[] = {0, 1, 3, 1, 2, 3};

        glBindVertexArray(debugData->vaoIds[0]);
        glBindBuffer(GL_ARRAY_BUFFER, debugData->vboIds[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debugData->eboIds[0]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void *)offsetof(VertexData, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void *)offsetof(VertexData, coord));
    }

    // texture
    int width, height, nrChannels;
    char *path = resolve_stack("fonts/consolas.png");
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    stack_free(alloc->stack, (void *)path);
    if (data == NULL)
    {
        printf("debug: failed to load font \n");
        debugData->enabled = 0;
        return;
    }

    glGenTextures(1, debugData->fontTexture);
    glBindTexture(GL_TEXTURE_2D, debugData->fontTexture[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
}

static Vec2 calculateSpace(const char *str, Vec2 bound)
{
    Vec2 v = {0, 0};
    Vec3 of = {0, 0, 0};
    int i = 0;
    while (*str != '\0')
    {
        if (*str == '\n')
        {
            of.x = 0;
            of.y += bound.y;
            str++;
        }
        of.x += bound.x;
        str++;
        i++;

        if (of.x > v.x)
            v.x = of.x;
        if (of.y > v.y)
            v.y = of.y;
    }
    if (i > 0)
        v.y += bound.y;
    return v;
}

void debug_render()
{
    if (!debugData->enabled)
        return;

    if (debugData->count2d == 0 && debugData->count3d == 0)
        return;

    shader_begin(debugData->shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, debugData->fontTexture[0]);

    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LESS);
    glBlendEquation(GL_ADD);

    if (debugData->count2d > 0)
    {
        Mat4 ortho = mat4_orthographic(0, game->size.x, game->size.y, 0, -1.0f,
                                       1.0f);

        shader_mat4(debugData->shader, "projection", &ortho);
        shader_mat4(debugData->shader, "view", &mat4_identity);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        for (int i = 0; i < debugData->count2d; i++)
        {
            Text2DData it = debugData->data2d[i];
            Mat4 mt = mat4_scalef(it.scale);
            mt = mat4_mul(mt,
                          mat4_origin(vec3(it.position.x, it.position.y, 0)));
            shader_mat4(debugData->shader, "world", &mt);
            shader_vec4(debugData->shader, "color", &it.color);

            Vec2 space = calculateSpace(it.text, debugData->bound2d);
            space.x *= it.origin.x;
            space.y *= it.origin.y;
            Vec3 of = {-space.x, -space.y, 0};
            while (*it.text != '\0')
            {
                if (*it.text == '\n')
                {
                    of.x = -space.x;
                    of.y += debugData->bound2d.y;
                    it.text++;
                }
                shader_vec3(debugData->shader, "offset", &of);
                shader_int(debugData->shader, "character", (int)(*it.text));

                glBindVertexArray(debugData->vaoIds[1]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                of.x += debugData->bound2d.x;
                it.text++;
            }
        }
    }

    if (debugData->count3d > 0)
    {
        shader_mat4(debugData->shader, "projection", &camera->projection);
        shader_mat4(debugData->shader, "view", &camera->view);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        for (int i = 0; i < debugData->count3d; i++)
        {
            Text3DData it = debugData->data3d[i];

            Mat4 mt = mat4_scalef(it.scale);
            if (!rot_eq(it.rotation, rot_zero))
                mt = mat4_mul(mt, rot_matrix(it.rotation, vec3_zero));

            mt = mat4_mul(mt, mat4_origin(it.position));

            shader_mat4(debugData->shader, "world", &mt);
            shader_vec4(debugData->shader, "color", &it.color);

            Vec2 space = calculateSpace(it.text, debugData->bound3d);
            space.x *= it.origin.x;
            space.y *= it.origin.y;
            Vec3 of = {0, space.x, space.y};
            while (*it.text != '\0')
            {
                if (*it.text == '\n')
                {
                    of.y = space.x;
                    of.z -= debugData->bound3d.y;
                    it.text++;
                }
                shader_vec3(debugData->shader, "offset", &of);
                shader_int(debugData->shader, "character", (int)(*it.text));

                glBindVertexArray(debugData->vaoIds[0]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                of.y -= debugData->bound3d.x;
                it.text++;
            }
        }
    }

    shader_end();

    debugData->count2d = 0;
    debugData->count3d = 0;
    arena_reset(debugData->arena);
}

void debug_terminate()
{
    glDeleteVertexArrays(2, debugData->vaoIds);
    glDeleteBuffers(2, debugData->vboIds);
    glDeleteBuffers(2, debugData->eboIds);
    glDeleteTextures(1, debugData->fontTexture);
    shader_destroy(debugData->shader);
}

void debug_origin(Vec2 origin)
{
    debugData->origin = origin;
}

void debug_color(Color color)
{
    debugData->color = color;
}

void debug_rotation(Rot rot)
{
    debugData->rotation = rot;
}

void debug_scale(float scale)
{
    debugData->scale = scale;
}
void debug_reset()
{
    debugData->origin = vec2_zero;
    debugData->color = color_white;
    debugData->scale = 1.0f;
    debugData->rotation = rot_zero;
}

void debug_string(Vec2 pos, const char *str, int n)
{
    if (debugData->count2d == max_elements)
        debugData->count2d = 0;

    char *cpy = arena_alloc(debugData->arena, n);
    memcpy(cpy, str, n);
    Text2DData dt;
    dt.position = pos;
    dt.text = cpy;
    dt.scale = debugData->scale;
    dt.origin = debugData->origin;
    dt.color = debugData->color;

    debugData->data2d[debugData->count2d++] = dt;
}

void debug_string3d(Vec3 pos, const char *str, int n)
{
    if (debugData->count3d == max_elements)
        debugData->count3d = 0;

    char *cpy = arena_alloc(debugData->arena, n);
    memcpy(cpy, str, n);
    Text3DData dt;
    dt.position = pos;
    dt.text = cpy;
    dt.scale = debugData->scale;
    dt.rotation = debugData->rotation;
    dt.origin = debugData->origin;
    dt.color = debugData->color;

    debugData->data3d[debugData->count3d++] = dt;
}

void debug_stringf(Vec2 pos, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    char *buffer = (char *)stack_alloc(alloc->stack, len + 1);

    if (buffer != NULL)
    {
        va_start(args, fmt);
        vsnprintf(buffer, len + 1, fmt, args);
        va_end(args);
        buffer[len] = '\0';
        debug_string(pos, buffer, len + 1);
    }

    stack_free(alloc->stack, (void *)buffer);
}

void debug_string3df(Vec3 pos, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    char *buffer = (char *)stack_alloc(alloc->stack, len + 1);

    if (buffer != NULL)
    {
        va_start(args, fmt);
        vsnprintf(buffer, len + 1, fmt, args);
        va_end(args);
        buffer[len] = '\0';
        debug_string3d(pos, buffer, len + 1);
    }
    stack_free(alloc->stack, (void *)buffer);
}