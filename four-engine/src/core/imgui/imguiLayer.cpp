#include "four-pch.h"

#include "core/engine.hpp"
#include "core/imgui/imguiLayer.hpp"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>


namespace four
{
bool ImGuiLayer::Init()
{
  assert(false && "not yet implemented");

  LOG_CORE_INFO("On ImGuiLayer Init.");

  // // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  if (!ImGui_ImplGlfw_InitForVulkan(Engine::Get()->GetWindow()->GetWindow(), false))
  {
    LOG_CORE_ERROR("ImGui_ImplGlfw_InitForVulkan failed.");
    return false;
  }

  // TODO: init vulkan for imgui setup reqired

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

void ImGuiLayer::begin()
{
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiLayer::OnImGuiRender()
{
  ImGui::ShowDemoWindow();
}

void ImGuiLayer::end()
{
  ImGui::Render();
}

void ImGuiLayer::OnUpdate()
{
  begin();
  OnImGuiRender();
  end();
}

void ImGuiLayer::Shutdown()
{
  LOG_CORE_INFO("On ImGuiLayer Shutdown.");
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}


} // namespace four
