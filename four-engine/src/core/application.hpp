#pragma once

#include "core/imgui/imguiLayer.hpp"
#include "core/layerStack.hpp"
#include "window/sdl/sdlWindow.hpp"

namespace four
{

class Application
{
public:
  explicit Application(std::string_view title, uint32_t width, uint32_t height) :
  m_Window(Window<SdlWindow>::CreateWindow(title, width, height))
  {
    m_Window->SetEventCallBack(WindowCloseEvent(), [&]() { OnExit(); });
    m_Window->SetEventCallBack(WindowResizeEvent(), [&](uint32_t width, uint32_t height) { OnResize(width, height); });

    m_ImGuiLayer.PushLayer(std::make_unique<ImGuiLayer>());
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
