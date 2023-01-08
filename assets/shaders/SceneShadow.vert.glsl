#version 460 core

layout(location = 0) in vec3 a_pos;

layout(location = 0) out vec3 v_position;

layout(binding = 0, std140) uniform FrameUniforms
{
  mat4 viewProj;
  mat4 invViewProj;
  mat4 sunViewProj;
  vec3 sunDir;
  vec3 sunIntensity;
  vec3 cameraPos;
};

struct ObjectUniforms
{
  mat4 model;
  vec3 tint;
};

layout(binding = 1, std430) readonly buffer SSBO0
{
  ObjectUniforms objects[];
};

void main()
{
  int i = gl_InstanceID;
  v_position = (objects[i].model * vec4(a_pos, 1.0)).xyz;
  gl_Position = viewProj * vec4(v_position, 1.0);
}