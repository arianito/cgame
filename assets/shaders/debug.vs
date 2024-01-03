#version 330 core

uniform mat4 projection;
uniform mat4 view;
uniform mat4 world;
uniform vec3 offset;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
  gl_Position = projection * view * world * vec4(aPos + offset, 1.0);
  TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}