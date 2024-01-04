#version 330 core

uniform mat4 view_projection;
uniform mat4 world;
uniform vec2 origin;
uniform float basis;
uniform vec2 tex_size;
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_coords;
out vec2 b_coords;

void main() {
  b_coords = a_coords;
  vec2 size = vec2(basis, basis * (tex_size.y / tex_size.x));
  vec2 p = ((a_pos.yz + (origin - 0.5)) * size);
  
  gl_Position = view_projection * world * vec4(0, p, 1);
}