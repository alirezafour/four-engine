#include "four-pch.h"


#include "core/engine.hpp"
#include "core/application.hpp"

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
  m_Renderer = std::make_unique<Renderer>(*m_Window);
  m_Renderer->Init();
}

//====================================================================================================
Engine::~Engine()
{
  m_ImGuiLayer.Shutdown();
  m_Renderer->Shutdown();
  m_Window->Shutdown();
  Log::Shutdown();
}

//====================================================================================================
void Engine::Run()
{
  try
  {
    m_LastFrameTimePoint = std::chrono::high_resolution_clock::now();
    while (!m_Window->ShouldClose())
    {
      const auto startTime = std::chrono::high_resolution_clock::now();
      const auto frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(startTime - m_LastFrameTimePoint);
      m_LastFrameTimePoint = startTime;

      m_Window->OnUpdate();
      if (m_Application != nullptr)
      {
        m_Application->OnUpdate(static_cast<float>(frameTime.count()) / 1000.0f);
      }
      m_ImGuiLayer.OnUpdate();

      const auto endTime  = std::chrono::high_resolution_clock::now();
      const auto realTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

      if (const auto sleepTime = TargetFrameTime - realTime.count(); sleepTime > 0)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
      }
      const float fps = 1000.0f / static_cast<float>(frameTime.count());
      LOG_INFO("FPS: {}, ms: {}, realtime: {}", fps, frameTime.count() / 1000.f, realTime.count() / 1000.f);
    }
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR("Exception: {}", e.what());
  }
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

} // namespace four
