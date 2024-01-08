#version 330 core

in vec2 TexCoord;

uniform sampler2D texture1;

uniform vec2 tex_size;
uniform int pixelart = 1;
uniform vec4 crop = vec4(0);
uniform float threshold = 0.5;

vec4 blinear(vec2 uv) {
  return texture(texture1, uv);
}

vec4 pixel_art(vec2 uv) {
  vec2 box_size = clamp(fwidth(uv) * tex_size, 1e-5, 1);
  vec2 tx = uv * tex_size - 0.5 * box_size;
  vec2 ofc = smoothstep(1 - box_size, vec2(1), fract(tx));
  vec2 finaluv = (floor(tx) + 0.5 + ofc) / tex_size;
  return textureGrad(texture1, finaluv, dFdx(finaluv), dFdy(finaluv));
}

vec2 crop_texture(vec2 uv, vec2 tex_size, vec4 crop) {
  return crop.xy / tex_size + (uv * (crop.zw / tex_size));
}

void main() {
  vec4 texel = vec4(0);
  vec2 uv = TexCoord;

  if(crop.z != 0 && crop.w != 0)
    uv = crop_texture(uv, tex_size, crop);

  if(pixelart == 1)
    texel = pixel_art(uv);
  else
    texel = blinear(uv);
  
  if(texel.a < threshold) discard;

  gl_FragColor = texel;
}