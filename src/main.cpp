#include <Application.h>
#include <Fwog/Pipeline.h>
#include <Fwog/Rendering.h>
#include <Fwog/Texture.h>
#include <Fwog/Buffer.h>
#include <Fwog/Shader.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <optional>
#include <vector>

////////////////////////////////// Types
struct Vertex
{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texcoord;
};

struct FrameUniforms
{
  glm::mat4 viewProj;
  glm::mat4 invViewProj;
  glm::mat4 sunViewProj;
  glm::vec3 sunDir;
  uint32_t _padding00;
  glm::vec3 sunIntensity;
  uint32_t _padding01;
  glm::vec3 cameraPos;
  uint32_t _padding02;
  glm::ivec2 resolution;
};

struct ObjectUniforms
{
  glm::mat4 model;
};

float GetHeight([[maybe_unused]] glm::vec2 pos)
{
  return 0;
}

glm::vec3 GetGradient([[maybe_unused]] glm::vec2 pos)
{
  return {0, 1, 0};
}

std::vector<Vertex> CreateHeightmap(int size)
{
  std::vector<Vertex> vertices;
  vertices.reserve(size * size);

  for (int y = 0; y < size; y++)
  {
    for (int x = 0; x < size; x++)
    {
      // Position of the center of this cell w.r.t. the grid (in [-0.5, 0.5])
      glm::vec2 centerNorm = glm::vec2(x + 0.5f, y + 0.5f) / float(size) - 0.5f;
      float edgeOffset = 0.5f / size;

      // Make a quad of two triangles
      auto tlvp = centerNorm + glm::vec2(-edgeOffset, edgeOffset);
      Vertex tlv{
        .position = {tlvp.x, GetHeight(tlvp), tlvp.y},
        .normal = GetGradient(tlvp),
        .texcoord{},
      };
      auto trvp = centerNorm + glm::vec2(edgeOffset, edgeOffset);
      Vertex trv{
        .position = {trvp.x, GetHeight(trvp), trvp.y},
        .normal = GetGradient(trvp),
        .texcoord{},
      };
      auto blvp = centerNorm + glm::vec2(-edgeOffset, -edgeOffset);
      Vertex blv{
        .position = {blvp.x, GetHeight(blvp), blvp.y},
        .normal = GetGradient(blvp),
        .texcoord{},
      };
      auto brvp = centerNorm + glm::vec2(edgeOffset, -edgeOffset);
      Vertex brv{
        .position = {brvp.x, GetHeight(brvp), brvp.y},
        .normal = GetGradient(brvp),
        .texcoord{},
      };

      vertices.push_back(tlv);
      vertices.push_back(blv);
      vertices.push_back(trv);

      vertices.push_back(brv);
      vertices.push_back(trv);
      vertices.push_back(blv);
    }
  }

  return vertices;
}

////////////////////////////////// Graphics

static constexpr std::array<Fwog::VertexInputBindingDescription, 3> sceneInputBindingDescs{
  Fwog::VertexInputBindingDescription{
    .location = 0,
    .binding = 0,
    .format = Fwog::Format::R32G32B32_FLOAT,
    .offset = offsetof(Vertex, position),
  },
  Fwog::VertexInputBindingDescription{
    .location = 1,
    .binding = 0,
    .format = Fwog::Format::R32G32B32_FLOAT,
    .offset = offsetof(Vertex, normal),
  },
  Fwog::VertexInputBindingDescription{
    .location = 2,
    .binding = 0,
    .format = Fwog::Format::R32G32_FLOAT,
    .offset = offsetof(Vertex, texcoord),
  },
};

Fwog::GraphicsPipeline CreateHeightmapDeferredPipeline()
{
  auto vs = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, Application::LoadFile("assets/shaders/Scene.vert.glsl"));
  auto fs = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, Application::LoadFile("assets/shaders/HeightmapDeferred.frag.glsl"));
  return Fwog::GraphicsPipeline({
    .vertexShader = &vs,
    .fragmentShader = &fs,
    .vertexInputState = {sceneInputBindingDescs},
    .rasterizationState = {.cullMode = Fwog::CullMode::NONE},
    .depthState = {.depthTestEnable = true, .depthWriteEnable = true},
  });
}

Fwog::GraphicsPipeline CreateShadeDeferredPipeline()
{
  auto vs = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, Application::LoadFile("assets/shaders/FullscreenTri.vert.glsl"));
  auto fs = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, Application::LoadFile("assets/shaders/ShadeDeferred.frag.glsl"));
  return Fwog::GraphicsPipeline({
    .vertexShader = &vs,
    .fragmentShader = &fs,
    .rasterizationState = {.cullMode = Fwog::CullMode::NONE},
  });
}

Fwog::GraphicsPipeline CreateShadowPipeline()
{
  auto vs = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, Application::LoadFile("assets/shaders/Scene.vert.glsl"));
  return Fwog::GraphicsPipeline({
    .vertexShader = &vs,
    .vertexInputState = {sceneInputBindingDescs},
    .depthState = {.depthTestEnable = true, .depthWriteEnable = true},
  });
}

Fwog::GraphicsPipeline CreateTexturePipeline()
{
  auto vs = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, Application::LoadFile("assets/shaders/FullscreenTri.vert.glsl"));
  auto fs = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, Application::LoadFile("assets/shaders/Texture.frag.glsl"));
  return Fwog::GraphicsPipeline({
    .vertexShader = &vs,
    .fragmentShader = &fs,
    .rasterizationState = {.cullMode = Fwog::CullMode::NONE},
  });
}

class GameApplication final : public Application
{
public:
  GameApplication(const Application::CreateInfo& createInfo);

private:
  void OnWindowResize(uint32_t newWidth, uint32_t newHeight) override;
  void OnUpdate(double dt) override;
  void OnRender(double dt) override;
  void OnGui(double dt) override;

  struct Frame
  {
    std::optional<Fwog::Texture> gAlbedo;
    std::optional<Fwog::Texture> gNormal;
    std::optional<Fwog::Texture> gDepth;
    std::optional<Fwog::Texture> shadingImage;
  };
  Frame frame;

  Fwog::TypedBuffer<FrameUniforms> frameUniformsBuffer;

  Fwog::GraphicsPipeline heightmapDeferredPipeline;
  Fwog::GraphicsPipeline shadeDeferredPipeline;
  Fwog::GraphicsPipeline shadowPipeline;
  Fwog::GraphicsPipeline texturePipeline;

  std::optional<Fwog::TypedBuffer<Vertex>> heightmapMesh;
};

GameApplication::GameApplication(const Application::CreateInfo& createInfo) 
  : Application(createInfo),
    frameUniformsBuffer(Fwog::BufferStorageFlag::DYNAMIC_STORAGE),
    heightmapDeferredPipeline(CreateHeightmapDeferredPipeline()),
    shadeDeferredPipeline(CreateShadeDeferredPipeline()),
    shadowPipeline(CreateShadowPipeline()),
    texturePipeline(CreateTexturePipeline())
{
  heightmapMesh = Fwog::TypedBuffer<Vertex>(CreateHeightmap(10));

  OnWindowResize(windowWidth, windowHeight);
}

void GameApplication::OnWindowResize([[maybe_unused]] uint32_t newWidth, [[maybe_unused]] uint32_t newHeight)
{
  frame.gAlbedo = Fwog::CreateTexture2D({newWidth, newHeight}, Fwog::Format::R8G8B8A8_UNORM);
  frame.gNormal = Fwog::CreateTexture2D({newWidth, newHeight}, Fwog::Format::R16G16B16_SNORM);
  frame.gDepth = Fwog::CreateTexture2D({newWidth, newHeight}, Fwog::Format::D32_UNORM);
  frame.shadingImage = Fwog::CreateTexture2D({newWidth, newHeight}, Fwog::Format::R16G16B16_FLOAT);
}

void GameApplication::OnUpdate([[maybe_unused]] double dt) {}

void GameApplication::OnRender([[maybe_unused]] double dt)
{
  auto ss = Fwog::SamplerState{.minFilter = Fwog::Filter::NEAREST, .magFilter = Fwog::Filter::NEAREST};
  auto nearestSampler = Fwog::Sampler(ss);

  auto sunIntensity = glm::vec3(1);
  auto sunDir = glm::normalize(glm::vec3(-.5f, -1.f, .5f));
  auto sunView = glm::lookAt(-sunDir * 50.f, glm::vec3(0), glm::vec3(0, 1, 0));
  auto sunProj = glm::ortho(-50.f, 50.f, -50.f, 50.f, 0.f, 100.f);
  auto sunViewProj = sunProj * sunView;

  auto projection = glm::perspective(glm::radians(60.0f), (float)windowWidth / windowHeight, 0.3f, 100.0f);
  auto viewProj = projection * mainCamera.GetViewMatrix();
  auto frameUniforms = FrameUniforms{
    .viewProj = viewProj,
    .invViewProj = glm::inverse(viewProj),
    .sunViewProj = sunViewProj,
    .sunDir = sunDir,
    .sunIntensity = sunIntensity,
    .cameraPos = mainCamera.position,
    .resolution = {windowWidth, windowHeight}
  };

  frameUniformsBuffer.SubDataTyped(frameUniforms);

  auto albedoAttachment = Fwog::RenderAttachment{&frame.gAlbedo.value()};
  auto normalAttachment = Fwog::RenderAttachment{&frame.gNormal.value()};
  auto gBufferAttachments = {albedoAttachment, normalAttachment};
  auto depthAttachment = Fwog::RenderAttachment{
    .texture = &frame.gDepth.value(),
    .clearValue = Fwog::ClearDepthStencilValue{.depth = 1.0f},
    .clearOnLoad = true,
  };

  auto terrainUniforms = ObjectUniforms{.model = glm::scale(glm::mat4(1), glm::vec3(100))};
  auto terrainUniformsBuffer = Fwog::TypedBuffer<ObjectUniforms>(terrainUniforms);

  Fwog::BeginRendering({
    .name = "Heightmap",
    .colorAttachments = {gBufferAttachments},
    .depthAttachment = &depthAttachment,
  });
  Fwog::Cmd::BindGraphicsPipeline(heightmapDeferredPipeline);
  Fwog::Cmd::BindVertexBuffer(0, heightmapMesh.value(), 0, sizeof(Vertex));
  Fwog::Cmd::BindUniformBuffer(0, frameUniformsBuffer, 0, frameUniformsBuffer.Size());
  Fwog::Cmd::BindStorageBuffer(0, terrainUniformsBuffer, 0, terrainUniformsBuffer.Size());
  Fwog::Cmd::Draw(6 * 10 * 10, 1, 0, 0);
  Fwog::EndRendering();

  auto shadingAttachment = Fwog::RenderAttachment(&frame.shadingImage.value());
  Fwog::BeginRendering({
    .name = "Shade",
    .colorAttachments = std::span(&shadingAttachment, 1),
  });
  Fwog::Cmd::BindGraphicsPipeline(shadeDeferredPipeline);
  Fwog::Cmd::BindSampledImage(0, frame.gAlbedo.value(), nearestSampler);
  Fwog::Cmd::BindSampledImage(1, frame.gNormal.value(), nearestSampler);
  Fwog::Cmd::BindSampledImage(2, frame.gDepth.value(), nearestSampler);
  Fwog::Cmd::BindUniformBuffer(0, frameUniformsBuffer, 0, frameUniformsBuffer.Size());
  Fwog::Cmd::Draw(3, 1, 0, 0);
  Fwog::EndRendering();

  Fwog::BeginSwapchainRendering({
    .viewport = {0, 0, windowWidth, windowHeight},
    .clearColorOnLoad = true,
    .clearColorValue = {0.1f, 0.f, 0.1f, 1.f},
  });
  Fwog::Cmd::BindGraphicsPipeline(texturePipeline);
  Fwog::Cmd::BindSampledImage(0, frame.shadingImage.value(), nearestSampler);
  Fwog::Cmd::Draw(3, 1, 0, 0);
  Fwog::EndRendering();
}

void GameApplication::OnGui([[maybe_unused]] double dt)
{
  ImGui::Begin("asdf");
  ImGui::Text("bruh");
  ImGui::End();
}

int main()
{
  auto appInfo = Application::CreateInfo{.name = "asdf"};
  auto app = GameApplication(appInfo);
  app.Run();
}
