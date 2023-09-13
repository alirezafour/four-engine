#pragma once

#include "core/imgui/imguiLayer.hpp"
#include "core/layer.hpp"
#include "core/layerStack.hpp"
#include "core/log.hpp"
#include "window/sdl/sdlWindow.hpp"
#include "window/window.hpp"
#include <memory>
#include <vector>

namespace four
{

class Application
{
public:
  explicit Application()
  {
    for (std::size_t i = 0; i < 10; ++i)
    {
      m_ImGuiLayer.PushLayer(std::make_unique<ImGuiLayer>());
    }
  }

  void OnEvent()
  {
    LOG_CORE_INFO("Application OnEvent.");
  }

private:
  LayerStack<ImGuiLayer>             m_ImGuiLayer;
  std::unique_ptr<Window<SdlWindow>> m_Window;
};
} // namespace four
