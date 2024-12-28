#include "four-pch.hpp"

#include "core/engine.hpp"
#include "core/application.hpp"
#include "imgui.h"

namespace four
{

std::unique_ptr<Engine> Engine::sm_Instance = nullptr;

//====================================================================================================
Engine::Engine(std::string_view title, u32 width, u32 height) :
m_Window{WindowType::CreateWindow(title, width, height)},
m_Renderer{m_Window},
m_Application{nullptr}
{
}

//====================================================================================================
Engine::~Engine() = default;

//====================================================================================================
void Engine::Run()
{
  try
  {
    auto lastFrameTimePoint = std::chrono::high_resolution_clock::now();
    u32  fps                = 0;
    while (!m_Window.ShouldClose())
    {
      const auto startTime     = std::chrono::high_resolution_clock::now();
      const auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(startTime - lastFrameTimePoint);
      lastFrameTimePoint       = startTime;

      m_Window.OnUpdate();
      if (m_Application != nullptr)
      {
        m_Application->OnUpdate(static_cast<f32>(frameDuration.count()) / 1000.0F);
      }

      const auto renderTime = std::chrono::high_resolution_clock::now();
      m_Renderer.Render();
      const auto renderTimeDuration = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - renderTime);

      const auto realTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::high_resolution_clock::now() - startTime)
                              .count();

      if (FrameCapEnabled)
      {
        if (const auto sleepTime = TargetFrameTime - static_cast<f32>(realTime); sleepTime > 0.0F)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<i32>(sleepTime)));
        }
      }
      fps = static_cast<u32>(1000.0F / static_cast<float>(frameDuration.count()));
      // LOG_CORE_INFO("FPS: {}, time: {}ms, realtime: {}ms, renderTime: {}us",
      //               fps,
      //               frameDuration.count(),
      //               realTime,
      //               renderTimeDuration.count());
    }
    m_Renderer.StopRender();
    Shutdown();
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR("Exception: {}", e.what());
  }
}

//====================================================================================================
void Engine::OnResize(u32 width, u32 height)
{
  LOG_CORE_INFO("resize: w: {}, h {}", width, height);
}

//====================================================================================================
void Engine::Shutdown()
{
  sm_Instance.reset();
}

} // namespace four
