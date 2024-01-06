#ifndef cgame_SHADER_H
#define cgame_SHADER_H

#include <stdint.h>

typedef uint32_t Shader;

Shader shader_create(const char *vs, const char *fs);
Shader shader_load(const char *vs, const char *fs);
void shader_destroy(Shader p);
void shader_begin(Shader p);
void shader_end();
void shader_texture(Shader p, const char *name, int a);
void shader_int(Shader p, const char *name, int a);
void shader_vec2(Shader p, const char *name, const void *v);
void shader_vec3(Shader p, const char *name, const void *v);
void shader_vec4(Shader p, const char *name, const void *v);
void shader_mat4(Shader p, const char *name, const void *v);
void shader_float(Shader p, const char *name, float f);

#endif