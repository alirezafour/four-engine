#include "four-pch.h"

#include "core/application.hpp"
#include "core/imgui/imguiLayer.hpp"
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"

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
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;


  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();

  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    style.WindowRounding              = 0.F;
    style.Colors[ImGuiCol_WindowBg].w = 1.F;
  }

  const char* glsl_version = "#version 100";

  auto* window = Application::GetInstance()->GetWindow();

  auto* sdlWindow = window->GetWindow();
  auto* glContext = window->GetGlContext();


  ImGui_ImplSDL3_InitForOpenGL(sdlWindow, glContext);
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
