#include "four-pch.h"

#include "core/imgui/imguiLayer.hpp"


namespace four
{
bool ImGuiLayer::Init()
{
  LOG_CORE_INFO("On ImGuiLayer Init.");
  return true;
}

void ImGuiLayer::OnEvent()
{
  LOG_CORE_INFO("On ImGuiLayer OnEvent.");
}

void ImGuiLayer::OnAttach()
{
  LOG_CORE_INFO("On ImGuiLayer OnAttach.");
}

void ImGuiLayer::OnDetach()
{
  LOG_CORE_INFO("On ImGuiLayer OnDetach.");
}

void ImGuiLayer::OnUpdate()
{
  // LOG_CORE_INFO("On ImGuiLayer Update.");
}

void ImGuiLayer::Shutdown()
{
  LOG_CORE_INFO("On ImGuiLayer Shutdown.");
}


} // namespace four
