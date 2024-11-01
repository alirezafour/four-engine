#include "four-pch.h"

#include "renderer/renderer.hpp"

#include "window/glfw/glfwWindow.hpp"

namespace four
{

//===============================================================================
Renderer::Renderer(Window<GlfwWindow>& window) :
m_Window{window},
m_VulkanContext{window},
m_VulkanRenderer{window, m_VulkanContext},
m_VulkanPipeline{m_VulkanContext}
{
  const bool result = Init();
  if (!result)
  {
    LOG_CORE_ERROR("Failed to initialize Renderer.");
  }
}

//===============================================================================
Renderer::~Renderer()
{
  Shutdown();
}

//===============================================================================
bool Renderer::Init()
{
  return true;
}

//===============================================================================
void Renderer::Shutdown()
{
}

} // namespace four
