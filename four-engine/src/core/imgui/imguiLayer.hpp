#pragma once

#include "core/layer.hpp"

namespace four
{

class ImGuiLayer : Layer<ImGuiLayer>
{
public:
  void OnAttach()
  {
    LOG_CORE_INFO("ImGuiLayer on Attach.");
  }
  void OnDetach()
  {
    LOG_CORE_INFO("ImGuiLayer on Detach.");
  }
  void OnEvent()
  {
    LOG_CORE_INFO("ImGuiLayer on Event.");
  }
};
} // namespace four
