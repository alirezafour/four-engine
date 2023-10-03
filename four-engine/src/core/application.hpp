#pragma once

#include "core/imgui/imguilayerstack.hpp"
#include "window/sdl/sdlWindow.hpp"

namespace four
{

class Application
{
public:
  static Application* Init(std::string_view title, uint32_t width, uint32_t height)
  {
    if (sm_Instance)
    {
      LOG_CORE_WARN("Application Already initialized!");
      return sm_Instance.get();
    }
    sm_Instance = std::move(std::unique_ptr<Application>(new Application(title, width, height)));
    return sm_Instance.get();
  }
  void Run();

  static Application* GetInstance()
  {
    return sm_Instance.get();
  }

  [[nodiscard]] inline auto* GetWindow() noexcept
  {
    return m_Window.get();
  }

  void Shutdown();

private:
  explicit Application(std::string_view title, uint32_t width, uint32_t height);
  void OnExit()
  {
    m_IsRunning = false;
  }

  bool InitWindow(std::string_view title, uint32_t width, uint32_t height);
  bool InitLog();

  void OnResize(uint32_t width, uint32_t height);


  static std::unique_ptr<Application> sm_Instance;
  ImGuiLayerStack                     m_ImGuiLayer;
  std::unique_ptr<SdlWindow>          m_Window;
  bool                                m_IsRunning = true;
};
} // namespace four
