#pragma once

#include "core/imgui/imguiLayer.hpp"
#include "core/layer.hpp"
#include "core/layerStack.hpp"
#include "core/log.hpp"
#include "window/sdl/sdlWindow.hpp"
#include "window/window.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace four
{

class Application
{
public:
  explicit Application() : m_Window(Window<SdlWindow>::CreateWindow("title", 800, 600))
  {
    m_Window->GetEvent<WindowCloseEvent>().SetupCallBack([&]() { OnExit(); });
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

private:
  LayerStack<ImGuiLayer>     m_ImGuiLayer;
  std::unique_ptr<SdlWindow> m_Window;
  bool                       m_IsRunning = true;
};
} // namespace four
