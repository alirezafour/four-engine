#pragma once

#include "core/imgui/imguiLayer.hpp"

class TempImGuiLayer : public four::ImGuiLayer
{
public:
  TempImGuiLayer(const TempImGuiLayer&)            = default;
  TempImGuiLayer(TempImGuiLayer&&)                 = default;
  TempImGuiLayer& operator=(const TempImGuiLayer&) = default;
  TempImGuiLayer& operator=(TempImGuiLayer&&)      = default;
  ~TempImGuiLayer() override                       = default;
};
