#pragma once

#include "core/imgui/imguiLayer.hpp"
#include "core/layerStack.hpp"
#include "core/log.hpp"
#include "window/sdl/sdlWindow.hpp"

namespace four
{

class Application
{
public:
  explicit Application() : m_Window(Window<SdlWindow>::CreateWindow("title", 800, 600))
  {
    m_Window->GetEvent<WindowCloseEvent>().SetupCallBack([&]() { OnExit(); });
    m_Window->GetEvent<WindowResizeEvent>().SetupCallBack([&](uint32_t w, uint32_t h) { OnResize(w, h); });
  }

  void OnEvent()
  {
    LOG_CORE_INFO("Application OnEvent.");
  }

  void Run()
  {
    while (m_IsRunning)
    {
      m_Window->OnUpdate();
    }
  }

private:
  void OnExit()
  {
    m_IsRunning = false;
  }
  void OnResize(uint32_t width, uint32_t height)
  {
    LOG_CORE_INFO("resize: w: {}, h {}", width, height);
  }

private:
  LayerStack<ImGuiLayer>     m_ImGuiLayer;
  std::unique_ptr<SdlWindow> m_Window;
  bool                       m_IsRunning = true;
};
} // namespace four
