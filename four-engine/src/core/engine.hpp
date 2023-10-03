#pragma once

#include "core/core.hpp"

#include "core/imgui/imguiLayer.hpp"
#include "core/layerStack.hpp"
#include "window/sdl/sdlWindow.hpp"

namespace four
{

class Engine
{
public:
  static Engine* Init(std::string_view title, uint32_t width, uint32_t height)
  {
    if (sm_Instance)
    {
      LOG_CORE_WARN("Engine Already initialized.");
      return sm_Instance.get();
    }
    sm_Instance = std::unique_ptr<Engine>(new Engine(title, width, height));
    return sm_Instance.get();
  }

  static Engine* Get()
  {
    if (sm_Instance)
    {
      return sm_Instance.get();
    }

    LOG_CORE_ERROR("Engine is not initialized.");
    return nullptr;
  }

  void Run();

  [[nodiscard]] inline auto* GetWindow() noexcept
  {
    return m_Window.get();
  }

  void Shutdown();

private:
  explicit Engine(std::string_view title, uint32_t width, uint32_t height);
  void OnExit()
  {
    m_IsRunning = false;
  }

  bool InitWindow(std::string_view title, uint32_t width, uint32_t height);
  bool InitLog();
  bool InitImGuiLayerStack();

  void OnResize(uint32_t width, uint32_t height);

private:
  static std::unique_ptr<Engine> sm_Instance;
  LayerStack<ImGuiLayer>         m_ImGuiLayer;
  std::unique_ptr<SdlWindow>     m_Window;
  bool                           m_IsRunning = true;
};
} // namespace four
