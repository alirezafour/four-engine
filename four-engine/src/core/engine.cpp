#include "four-pch.h"

#include "core/engine.hpp"

namespace four
{
std::unique_ptr<Engine> Engine::sm_Instance = nullptr;

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
  m_VulkPipeline = std::make_unique<VulkPipeline>(*m_VulkDevice,
                                                  VulkPipeline::DefaultPipeLineConfigInfo(width, height),
                                                  "shaders/simpleShader.vert.spv",
                                                  "shaders/simpleShader.frag.spv");

  m_SwapChain = std::make_unique<VulkSwapChain>(*m_VulkDevice, m_Window->GetExtent());
}

Engine::~Engine()
{
  m_ImGuiLayer.Shutdown();
  m_VulkPipeline.reset();
  m_VulkDevice->Cleanup();
  m_Window->Shutdown();
  Log::Shutdown();
}

void Engine::Run()
{
  while (!m_Window->ShouldClose())
  {
    m_Window->OnUpdate();
    m_ImGuiLayer.OnUpdate();
  }
}

bool Engine::InitLog()
{
  return Log::Init();
}

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

void Engine::OnResize(uint32_t width, uint32_t height)
{
  LOG_CORE_INFO("resize: w: {}, h {}", width, height);
}

void Engine::Shutdown()
{
  sm_Instance.reset();
}

} // namespace four
