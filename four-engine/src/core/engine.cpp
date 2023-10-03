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
}

void Engine::Run()
{
  while (m_IsRunning)
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
  m_Window = std::move(Window<SdlWindow>::CreateWindow(title, width, height));
  // check if window created successfully
  if (m_Window == nullptr)
  {
    LOG_ERROR("Initializing Window falied");
    m_IsRunning = false;
    return false;
  }

  // setup required events
  m_Window->SetEventCallBack(WindowCloseEvent(), [&]() { OnExit(); });
  m_Window->SetEventCallBack(WindowResizeEvent(), [&](uint32_t width, uint32_t height) { OnResize(width, height); });
  return true;
}

void Engine::OnResize(uint32_t width, uint32_t height)
{
  LOG_CORE_INFO("resize: w: {}, h {}", width, height);
}

void Engine::Shutdown()
{
  m_IsRunning = false;
  m_ImGuiLayer.Shutdown();
  m_Window->Shutdown();
  Log::Shutdown();
}

} // namespace four
