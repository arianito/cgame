#ifndef cgame_SHADER_H
#define cgame_SHADER_H

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include "glad.h"

typedef GLint Shader;

Shader shader_create(const char *vs, const char *fs);

Shader shader_load(const char *vs, const char *fs);

void shader_destroy(Shader p);

static inline void shader_begin(Shader p)
{
    glUseProgram(p);
}

static inline void shader_end()
{
    glUseProgram(0);
}

static inline void shader_texture(Shader p, const char *name, int a)
{
    glUniform1i(glGetUniformLocation(p, name), a);
}

static inline void shader_int(Shader p, const char *name, int a)
{
    glUniform1i(glGetUniformLocation(p, name), a);
}

static inline void shader_vec2(Shader p, const char *name, const void *v)
{
    glUniform2fv(glGetUniformLocation(p, name), 1, (const float *)v);
}

static inline void shader_vec3(Shader p, const char *name, const void *v)
{
    glUniform3fv(glGetUniformLocation(p, name), 1, (const float *)v);
}

static inline void shader_vec4(Shader p, const char *name, const void *v)
{
    glUniform4fv(glGetUniformLocation(p, name), 1, (const float *)v);
}

static inline void shader_mat4(Shader p, const char *name, const void *v)
{
    glUniformMatrix4fv(glGetUniformLocation(p, name), 1, GL_FALSE, (const float *)v);
}

static inline void shader_float(Shader p, const char *name, float f)
{
    glUniform1f(glGetUniformLocation(p, name), f);
}

#endif