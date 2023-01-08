#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

layout(location = 0) out vec3 v_position;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;
layout(location = 3) out vec3 v_tint;

layout(binding = 0, std140) uniform FrameUniforms
{
  mat4 viewProj;
  mat4 invViewProj;
  mat4 sunViewProj;
  vec3 sunDir;
  vec3 sunIntensity;
  vec3 cameraPos;
  ivec2 resolution;
};

struct ObjectUniforms
{
  mat4 model;
  vec3 tint;
};

layout(binding = 0, std430) readonly buffer SSBO0
{
  ObjectUniforms objects[];
};

void main()
{
  int i = gl_InstanceID;
  v_position = (objects[i].model * vec4(a_pos, 1.0)).xyz;
  v_normal = normalize(inverse(transpose(mat3(objects[i].model))) * a_normal);
  v_tint = objects[i].tint;
  gl_Position = viewProj * vec4(v_position, 1.0);
}