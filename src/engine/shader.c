
#include "shader.h"
#include <stdio.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad.h"

#include "file.h"
#include "mem/alloc.h"

Shader shader_create(const char *vs, const char *fs)
{
    GLint status = GL_TRUE;
    char error_msg[1024];
    GLsizei read;
    GLuint vsp = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsp, 1, &vs, NULL);
    glCompileShader(vsp);
    glGetShaderiv(vsp, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        glGetShaderInfoLog(vsp, 1024, &read, error_msg);
        printf("vs error: %s", error_msg);
        return 0;
    }
    GLuint fsp = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsp, 1, &fs, NULL);
    glCompileShader(fsp);
    glGetShaderiv(fsp, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        glGetShaderInfoLog(fsp, 1024, &read, error_msg);
        printf("fs error: %s", error_msg);
        return 0;
    }
    GLint programId = glCreateProgram();
    glAttachShader(programId, vsp);
    glAttachShader(programId, fsp);
    glLinkProgram(programId);
    glGetProgramiv(programId, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        glGetProgramInfoLog(programId, 1024, &read, error_msg);
        printf("compile error: %s", error_msg);
        return 0;
    }
    glDetachShader(programId, vsp);
    glDetachShader(programId, fsp);
    glDeleteShader(vsp);
    glDeleteShader(fsp);

    return programId;
}

Shader shader_load(const char *vs, const char *fs)
{
    StrView vsf = readfile_stack(vs);
    StrView fsf = readfile_stack(fs);

    Shader sh = shader_create(vsf.string, fsf.string);

    xxfreestack(fsf.string);
    xxfreestack(vsf.string);

    return sh;
}

void shader_destroy(Shader p)
{
    glDeleteProgram(p);
}


void shader_begin(Shader p)
{
    glUseProgram(p);
}

void shader_end()
{
    glUseProgram(0);
}

void shader_texture(Shader p, const char *name, int a)
{
    glUniform1i(glGetUniformLocation(p, name), a);
}

void shader_int(Shader p, const char *name, int a)
{
    glUniform1i(glGetUniformLocation(p, name), a);
}

void shader_vec2(Shader p, const char *name, const void *v)
{
    glUniform2fv(glGetUniformLocation(p, name), 1, (const float *)v);
}

void shader_vec3(Shader p, const char *name, const void *v)
{
    glUniform3fv(glGetUniformLocation(p, name), 1, (const float *)v);
}

void shader_vec4(Shader p, const char *name, const void *v)
{
    glUniform4fv(glGetUniformLocation(p, name), 1, (const float *)v);
}

void shader_mat4(Shader p, const char *name, const void *v)
{
    glUniformMatrix4fv(glGetUniformLocation(p, name), 1, GL_FALSE, (const float *)v);
}

void shader_float(Shader p, const char *name, float f)
{
    glUniform1f(glGetUniformLocation(p, name), f);
}