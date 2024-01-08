#version 330 core

uniform mat4 view_projection;
uniform mat4 world;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 coords;

out vec4 FragPosition;
out vec3 Normal;
out vec2 TexCoord;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

vec4 calculateBone(vec3 vertexPos, ivec4 boneIds, vec4 weights) {
  vec4 totalPosition = vec4(0.0f);
  for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
  {
      if(boneIds[i] == -1) 
          continue;

      if(boneIds[i] >= MAX_BONES) 
        return vec4(vertexPos, 1.0f);

      vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(vertexPos,1.0f);
      totalPosition += localPosition * weights[i];
      vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * normal;
  }
  return totalPosition;
}

void main() {
  TexCoord = coords;

  FragPosition = world * vec4(pos, 1.0);

  Normal = mat3(transpose(inverse(world))) * normal;

  gl_Position = view_projection * FragPosition;
}