#version 460 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec3 v_tint;

layout(location = 0) out vec3 o_color;

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

void main()
{
  // TODO: make the thingy flicker
  o_color = v_tint;
}