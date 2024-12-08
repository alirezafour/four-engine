#include "four-pch.hpp"

#include "core/engine.hpp"
#include "core/application.hpp"

namespace four
{

std::unique_ptr<Engine> Engine::sm_Instance = nullptr;

//====================================================================================================
Engine::Engine(std::string_view title, uint32_t width, uint32_t height) :
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
    auto     lastFrameTimePoint = std::chrono::high_resolution_clock::now();
    uint32_t fps                = 0;
    while (!m_Window.ShouldClose())
    {
      const auto startTime     = std::chrono::high_resolution_clock::now();
      const auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(startTime - lastFrameTimePoint);
      lastFrameTimePoint       = startTime;

      m_Window.OnUpdate();
      if (m_Application != nullptr)
      {
        m_Application->OnUpdate(static_cast<float>(frameDuration.count()) / 1000.0F);
      }

      const auto renderTime = std::chrono::high_resolution_clock::now();
      m_Renderer.Render();
      const auto renderTimeDuration = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - renderTime);
      m_ImGuiLayer.OnUpdate();

      const auto realTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::high_resolution_clock::now() - startTime)
                              .count();

      if (FrameCapEnabled)
      {
        if (const auto sleepTime = TargetFrameTime - static_cast<float>(realTime); sleepTime > 0.0F)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleepTime)));
        }
      }
      fps = static_cast<uint32_t>(1000.0F / static_cast<float>(frameDuration.count()));
      LOG_CORE_INFO("FPS: {}, time: {}ms, realtime: {}ms, renderTime: {}us",
                    fps,
                    frameDuration.count(),
                    realTime,
                    renderTimeDuration.count());
    }
    m_Renderer.StopRender();
    Shutdown();
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR("Exception: {}", e.what());
  }
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
