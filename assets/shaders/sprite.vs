#version 330 core

uniform mat4 view_projection;
uniform mat4 world;

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_coords;

out vec4 FragPosition;
out vec3 Normal;
out vec2 TexCoord;


void main() {
  vec3 pos = a_pos;

  TexCoord = a_coords;

  FragPosition = world * vec4(pos, 1.0);

  Normal = mat3(transpose(inverse(world))) * a_normal;

  gl_Position = view_projection * FragPosition;
}