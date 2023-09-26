#pragma once

#include "core/layer.hpp"

namespace four
{

class ImGuiLayer : public Layer<ImGuiLayer>
{
public:
  bool Init();
  void Shutdown();
  void OnAttach();
  void OnDetach();
  void OnUpdate();
  void OnEvent();
};
} // namespace four
