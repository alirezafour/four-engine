#pragma once

#include "core/imgui/imguiLayer.hpp"
#include "core/layerStack.hpp"

namespace four
{

class ImGuiLayerStack : public LayerStack<ImGuiLayer>
{
public:
  void BeginRender();
  void EndRender();
};

} // namespace four
