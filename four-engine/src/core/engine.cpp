#include "four-pch.hpp"

#include "core/engine.hpp"
#include "core/application.hpp"

#include "window/glfw/glfwWindow.hpp"

namespace four
{

std::unique_ptr<Engine> Engine::sm_Instance = nullptr;

//====================================================================================================
Engine::Engine(std::string_view title, uint32_t width, uint32_t height) :
m_Window{title, width, height},
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
    auto  lastFrameTimePoint = std::chrono::high_resolution_clock::now();
    float fps                = 0.0f;
    while (!m_Window.ShouldClose())
    {
      const auto startTime     = std::chrono::high_resolution_clock::now();
      const auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(startTime - lastFrameTimePoint);
      lastFrameTimePoint       = startTime;

      m_Window.OnUpdate();
      if (m_Application != nullptr)
      {
        m_Application->OnUpdate(static_cast<float>(frameDuration.count()) / 1000000.0f);
      }

      const auto renderTime = std::chrono::high_resolution_clock::now();
      m_Renderer.Render();
      const auto renderTimeDuration = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - renderTime);
      m_ImGuiLayer.OnUpdate();

      const auto realTime = std::chrono::duration_cast<std::chrono::microseconds>(
                              std::chrono::high_resolution_clock::now() - startTime)
                              .count();

      if (FrameCapEnabled)
      {
        if (const auto sleepTime = TargetFrameTime - static_cast<float>(realTime); sleepTime > 0.0f)
        {
          std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(1000.0f * sleepTime)));
        }
      }
      fps = 1000000.0f / static_cast<float>(frameDuration.count());
      // LOG_CORE_INFO("FPS: {}, time: {}ms, realtime: {}ms, renderTime: {}ms",
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
void Engine::OnResize(uint32_t width, uint32_t height)
{
  LOG_CORE_INFO("resize: w: {}, h {}", width, height);
}

//====================================================================================================
void Engine::Shutdown()
{
}

} // namespace four
