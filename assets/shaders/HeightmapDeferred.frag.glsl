#version 460 core

layout(location = 0) out vec3 o_albedo;
layout(location = 1) out vec3 o_normal;

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

void main()
{
  // TODO: change the albedo depending on (lower areas = beach, higher areas are brighter to simulate less fog blocking the light?)
  o_albedo = vec3(.2, .6, .3);
  o_normal = normalize(v_normal);
}