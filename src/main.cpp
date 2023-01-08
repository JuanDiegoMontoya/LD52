#include <Application.h>
#include <Fwog/Pipeline.h>
#include <Fwog/Rendering.h>
#include <Fwog/Texture.h>
#include <Fwog/Buffer.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <optional>

///////////////////////////////// Types
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
};

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
    .format = Fwog::Format::R16G16B16_SNORM,
    .offset = offsetof(Vertex, normal),
  },
  Fwog::VertexInputBindingDescription{
    .location = 2,
    .binding = 0,
    .format = Fwog::Format::R32G32_FLOAT,
    .offset = offsetof(Vertex, texcoord),
  },
};

struct ObjectUniforms
{
  glm::mat4 model;
};

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
    std::optional<Fwog::Texture> gShadingImage;
  };
  Frame frame;

  Fwog::TypedBuffer<FrameUniforms> frameUniformsBuffer;
};

GameApplication::GameApplication(const Application::CreateInfo& createInfo) : Application(createInfo) {}

void GameApplication::OnWindowResize([[maybe_unused]] uint32_t newWidth, [[maybe_unused]] uint32_t newHeight)
{
  frame.gAlbedo = Fwog::CreateTexture2D({newWidth, newHeight}, Fwog::Format::R8G8B8A8_UNORM);
  frame.gNormal = Fwog::CreateTexture2D({newWidth, newHeight}, Fwog::Format::R16G16B16_SNORM);
  frame.gDepth = Fwog::CreateTexture2D({newWidth, newHeight}, Fwog::Format::D32_UNORM);
  frame.gShadingImage = Fwog::CreateTexture2D({newWidth, newHeight}, Fwog::Format::R16G16B16_FLOAT);
}

void GameApplication::OnUpdate([[maybe_unused]] double dt) {}

void GameApplication::OnRender([[maybe_unused]] double dt)
{
  Fwog::BeginSwapchainRendering({
    .viewport = {windowWidth, windowHeight},
    .clearColorOnLoad = true,
    .clearColorValue = {1.f, 0.f, 1.f, 1.f},
  });
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
