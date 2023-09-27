#include "four-pch.h"

#include "core/application.hpp"
#include "core/imgui/imguiLayer.hpp"

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

namespace four
{

bool ImGuiLayer::Init()
{
  LOG_CORE_INFO("On Init ImGuiLayer.");

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsLight();

  auto  fourWindow = Application::GetInstance()->GetWindow();
  auto  window     = fourWindow->GetWindow();
  auto* gl_context = fourWindow->GetGlContext();

  const char* glsl_version = "#version 100";

  // Setup Platform/Renderer backends
  ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);

  return true;
}

void ImGuiLayer::OnAttach()
{
  LOG_CORE_INFO("On Attack IMGUILayer.");
}

void ImGuiLayer::OnUpdate()
{
  ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.
  ImGui::Text("This is some useful text.");
  ImGuiIO& io = ImGui::GetIO();
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
  ImGui::End();
}

void ImGuiLayer::OnDetach()
{
  LOG_CORE_INFO("OnDetach ImGuiLayer.");
}

void ImGuiLayer::OnEvent()
{
  LOG_CORE_INFO("OnEvent ImGuiLayer");
}

void ImGuiLayer::Shutdown()
{
  LOG_CORE_INFO("OnShutdown IMGUILayer.");
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
}

} // namespace four
