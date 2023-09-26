#include "core/application.hpp"
#include "four-pch.h"

#include "core/imgui/imguilayerstack.hpp"

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl3.h"
#include "SDL_opengl.h"

namespace four
{

void ImGuiLayerStack::BeginRender()
{
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
}

void ImGuiLayerStack::EndRender()
{
  // Rendering
  ImGui::Render();
  ImGuiIO& io = ImGui::GetIO();
  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  auto* window = Application::GetInstance()->GetWindow()->GetWindow();
  SDL_GL_SwapWindow(window);
}

} // namespace four
