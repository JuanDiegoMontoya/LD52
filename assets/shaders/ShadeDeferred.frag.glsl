#version 460 core

layout(binding = 0) uniform sampler2D s_gAlbedo;
layout(binding = 1) uniform sampler2D s_gNormal;
layout(binding = 2) uniform sampler2D s_gDepth;
layout(binding = 3) uniform sampler2D s_shadowDepth;

layout(location = 0) in vec2 v_uv;

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

vec3 UnprojectUV(float depth, vec2 uv, mat4 invXProj)
{
  float z = depth * 2.0 - 1.0; // OpenGL Z convention
  vec4 ndc = vec4(uv * 2.0 - 1.0, z, 1.0);
  vec4 world = invXProj * ndc;
  return world.xyz / world.w;
}

float hash(vec2 n)
{ 
  return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

vec2 Hammersley(uint i, uint N)
{
  return vec2(float(i) / float(N), float(bitfieldReverse(i)) * 2.3283064365386963e-10);
}

float Shadow(vec4 clip, vec3 normal, vec3 lightDir)
{
  vec2 uv = clip.xy * .5 + .5;
  if (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1)
  {
    return 0;
  }

  // Analytically compute slope-scaled bias
  const float maxBias = 0.0008;
  const float quantize = 2.0 / (1 << 23);
  ivec2 res = textureSize(s_shadowDepth, 0);
  float b = 1.0 / max(res.x, res.y) / 2.0;
  float NoD = clamp(-dot(sunDir.xyz, normal), 0.0, 1.0);
  float bias = quantize + b * length(cross(-sunDir.xyz, normal)) / NoD;
  bias = min(bias, maxBias);

  float lightOcclusion = 0.0;

  const int SAMPLES = 4;
  for (int i = 0; i < SAMPLES; i++)
  {
    float viewDepth = clip.z * .5 + .5;

    vec2 xi = fract(Hammersley(i, SAMPLES) + hash(gl_FragCoord.xy));
    float r = xi.x;
    float theta = xi.y * 2.0 * 3.14159;
    vec2 offset = 0.002 * vec2(r * cos(theta), r * sin(theta));
    float lightDepth = textureLod(s_shadowDepth, uv + offset, 0).x;

    lightDepth += bias;
    
    if (lightDepth >= viewDepth)
    {
      lightOcclusion += 1.0;
    }
  }

  return lightOcclusion / SAMPLES;
}

void main()
{
  vec3 albedo = textureLod(s_gAlbedo, v_uv, 0.0).rgb;
  vec3 normal = textureLod(s_gNormal, v_uv, 0.0).xyz;
  float depth = textureLod(s_gDepth, v_uv, 0.0).x;

  if (depth == 1.0)
  {
    o_color = vec3(0);
    return;
  }

  vec3 worldPos = UnprojectUV(depth, v_uv, invViewProj);
  
  vec3 incidentDir = -sunDir.xyz;
  float cosTheta = max(0.0, dot(incidentDir, normal));
  vec3 diffuse = albedo * cosTheta * sunIntensity.rgb;
  diffuse *= Shadow(sunViewProj * vec4(worldPos, 1.0), normal, sunDir.xyz);

  vec3 ambient = vec3(1.03) * albedo;
  vec3 finalColor = diffuse + ambient;

  o_color = finalColor;
}