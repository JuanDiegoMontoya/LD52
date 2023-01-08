#version 460 core

layout(location = 0) out vec3 o_albedo;
layout(location = 1) out vec3 o_normal;

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec3 v_tint;

void main()
{
  o_albedo = v_tint;
  o_normal = normalize(v_normal);
}