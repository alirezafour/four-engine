#pragma once

#include "core/layer.hpp"

namespace four
{

class ImGuiLayer : public Layer<ImGuiLayer>
{
public:
  virtual ~ImGuiLayer() = default;

  bool         Init();
  void         Shutdown();
  void         OnAttach();
  void         OnDetach();
  void         OnUpdate();
  virtual void OnImGuiRender();
  void         OnEvent();

private:
  void begin();
  void end();
};
} // namespace four
