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
}

//====================================================================================================
Engine::~Engine()
{
}

//====================================================================================================
void Engine::Run()
{
  try
  {
    auto  lastFrameTimePoint = std::chrono::high_resolution_clock::now();
    float fps                = 0.0f;
    while (!m_Window->ShouldClose())
    {
      const auto startTime     = std::chrono::high_resolution_clock::now();
      const auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(startTime - lastFrameTimePoint);
      lastFrameTimePoint       = startTime;

      m_Window->OnUpdate();
      if (m_Application != nullptr)
      {
        m_Application->OnUpdate(static_cast<float>(frameDuration.count()) / 1000.0f);
      }

      const auto renderTime = std::chrono::high_resolution_clock::now();
      m_Renderer->Render();
      const auto renderTimeDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - renderTime);
      m_ImGuiLayer.OnUpdate();

      const auto realTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::high_resolution_clock::now() - startTime)
                              .count();

      if (FrameCapEnabled)
      {
        if (const auto sleepTime = TargetFrameTime - realTime; sleepTime > 0)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        }
      }
      fps = 1000.0f / static_cast<float>(frameDuration.count());
      LOG_INFO("FPS: {}, time: {}ms, realtime: {}ms, renderTime: {}ms",
               fps,
               frameDuration.count(),
               realTime,
               renderTimeDuration.count());
    }
    m_Renderer->StopRender();
    Shutdown();
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
  if (sm_Instance)
  {
    sm_Instance->m_Renderer.reset();
    sm_Instance->m_Window.reset();
  }
}

} // namespace four
