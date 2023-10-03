#include "four-pch.h"

#include "core/application.hpp"

namespace four
{

Application::Application(std::string_view title, uint32_t width, uint32_t height) : m_Window(nullptr), m_IsRunning(true)
{
  if (!InitLog() || !InitWindow(title, width, height))
  {
    // if failed to initialize revent running the app
    m_IsRunning = false;
    LOG_CORE_ERROR("Failed to Initialized in Application struct");
    return;
  }
  m_ImGuiLayer.PushLayer(std::make_unique<ImGuiLayer>());
}

void Application::Run()
{
  while (m_IsRunning)
  {
    m_Window->OnUpdate();
    m_ImGuiLayer.BeginRender();
    m_ImGuiLayer.OnUpdate();
    m_ImGuiLayer.EndRender();
  }
}

bool Application::InitLog()
{
  if (!Log::Init())
  {
    LOG_CORE_ERROR("Init Log Failed.");
    return false;
  }

  return true;
}

bool Application::InitWindow(std::string_view title, uint32_t width, uint32_t height)
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

void Application::OnResize(uint32_t width, uint32_t height)
{
  LOG_CORE_INFO("resize: w: {}, h {}", width, height);
}

void Application::Shutdown()
{
  m_IsRunning = false;
  m_ImGuiLayer.Shutdown();
  m_Window->Shutdown();
  Log::Shutdown();
}

} // namespace four
