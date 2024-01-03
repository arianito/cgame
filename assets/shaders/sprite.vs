#version 330 core

uniform mat4 projection;
uniform mat4 view;
uniform mat4 world;

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_coords;


void main() 
  gl_Position = projection * view * world * vec4(a_pos, 1.0);
}