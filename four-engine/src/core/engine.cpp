#include "four-pch.h"

#include "core/engine.hpp"
#include "renderer/simpleRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

namespace four
{


std::unique_ptr<Engine> Engine::sm_Instance = nullptr;

//====================================================================================================
Engine::Engine(std::string_view title, uint32_t width, uint32_t height) : m_Window(nullptr)
{
  if (!InitLog() || !InitWindow(title, width, height))
  {
    LOG_CORE_ERROR("Failed Initializing Engine.");
    return;
  }

  m_ImGuiLayer.PushLayer(std::make_unique<ImGuiLayer>());
  m_ImGuiLayer.PushLayer(std::make_unique<ImGuiLayer>());
  m_VulkDevice = std::make_unique<VulkDevice>(m_Window.get());
  m_VulkDevice->InitVulkan();
  m_Renderer = std::make_unique<Renderer>(*m_Window, *m_VulkDevice);

  LoadGameObjects();
}

//====================================================================================================
Engine::~Engine()
{
  m_ImGuiLayer.Shutdown();
  m_VulkDevice->Cleanup();
  m_Window->Shutdown();
  Log::Shutdown();
}

//====================================================================================================
void Engine::Run()
{
  try
  {
    SimpleRenderSystem simpleRenderSystem(*m_VulkDevice, m_Renderer->GetSwapChainRenderPass());

    while (!m_Window->ShouldClose())
    {
      m_Window->OnUpdate();
      if (auto* commandBuffer = m_Renderer->BeginFrame())
      {
        m_Renderer->BeginSwapChainRenderPass(commandBuffer);
        simpleRenderSystem.RenderGameObjects(commandBuffer, m_GameObjects);
        m_Renderer->EndSwapChainRenderPass(commandBuffer);
        m_Renderer->EndFrame();
      }
      m_ImGuiLayer.OnUpdate();
    }
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR("Exception: {}", e.what());
  }

  vkDeviceWaitIdle(m_VulkDevice->GetDevice());
}

//====================================================================================================
bool Engine::InitLog()
{
  return Log::Init();
}

//====================================================================================================
bool Engine::InitWindow(std::string_view title, uint32_t width, uint32_t height)
{
  m_Window.reset();
  if (m_Window = Window<UsingWindow>::CreateWindow(title, width, height); m_Window == nullptr)
  {
    LOG_ERROR("Initializing Window falied");
    return false;
  }

  // setup required events
  return true;
}

//====================================================================================================
void Engine::OnResize(uint32_t width, uint32_t height)
{
  LOG_CORE_INFO("resize: w: {}, h {}", width, height);
}

//====================================================================================================
void Engine::Shutdown()
{
  sm_Instance.reset();
}

//====================================================================================================
void Engine::LoadGameObjects()
{
  std::vector<VulkModel::Vertex> vertices = {{{0.0F, -0.5F}, {1.0F, 0.0F, 0.0F}},
                                             {{0.5F, 0.5F}, {0.0F, 1.0F, 0.0F}},
                                             {{-0.5F, 0.5F}, {0.0F, 0.0F, 1.0F}}};

  auto vulkModel = std::make_shared<VulkModel>(*m_VulkDevice, vertices);

  auto triangle = TempGameObj::Create();
  triangle.SetModel(vulkModel);
  triangle.SetColor(glm::vec3(0.1F, 0.8F, 0.1F));
  triangle.SetTransform2D({{0.2F, 0.0F}, {2.0F, 0.5F}, 0.25F * glm::two_pi<float>()});


  m_GameObjects.push_back(std::move(triangle));
}

} // namespace four
