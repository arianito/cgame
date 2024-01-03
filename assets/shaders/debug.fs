#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;
uniform int character;
uniform vec4 color;

void main() {
  float p = 1 / 16.0;
  vec2 coords = TexCoord * p;

  coords.x += p * (character % 16);
  coords.y += p * (character / 16);

  vec4 tex = texture(texture1, coords);
  FragColor = vec4(color.r, color.g, color.b, tex.r * color.a);
}