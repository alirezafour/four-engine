#pragma once

#include "core/imgui/imguiLayer.hpp"
#include "core/layerStack.hpp"
#include "window/sdl/sdlWindow.hpp"

namespace four
{

class Application
{
public:
  explicit Application(std::string_view title, uint32_t width, uint32_t height);
  void Run();

private:
  void OnExit()
  {
    m_IsRunning = false;
  }

  void Shutdown();
  bool InitWindow(std::string_view title, uint32_t width, uint32_t height);
  bool InitLog();

  void OnResize(uint32_t width, uint32_t height);

  LayerStack<ImGuiLayer>     m_ImGuiLayer;
  std::unique_ptr<SdlWindow> m_Window;
  bool                       m_IsRunning;
};
} // namespace four
