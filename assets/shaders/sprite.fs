#version 330 core

in vec2 b_coords;

uniform sampler2D texture1;

uniform int pixelart;
uniform vec2 tex_size;
uniform vec2 screen;
uniform vec4 crop;
uniform float threshold;

vec4 pixel_art(vec2 data_uv) {

  vec2 box_size = clamp(fwidth(data_uv) * tex_size, 1e-5, 1);
  vec2 tx = data_uv * tex_size - 0.5 * box_size;
  vec2 ofc = smoothstep(1 - box_size, vec2(1), fract(tx));
  vec2 uv = (floor(tx) + 0.5 + ofc) / tex_size;
  return textureGrad(texture1, uv, dFdx(uv), dFdy(uv));
}

vec4 blinear(vec2 data_uv) {
  return texture(texture1, data_uv);
}

void main() {
  vec2 data_uv = crop.xy / tex_size + (b_coords * (crop.zw / tex_size));
  vec4 texel = pixelart == 1 ? pixel_art(data_uv) : blinear(data_uv);
  if(texel.a < threshold) discard;
  gl_FragColor = texel;
}