#include "four-pch.h"

#include "renderer/renderer.hpp"

#include "window/glfw/glfwWindow.hpp"

namespace four
{

//===============================================================================
Renderer::Renderer(Window<GlfwWindow>& window) :
m_Window{window},
m_VulkanContext{window},
m_VulkanRenderer{window, m_VulkanContext}
{
}

//===============================================================================
Renderer::~Renderer()
{
  Shutdown();
}

//===============================================================================
bool Renderer::Init()
{
  bool result = false;
  try
  {
    result = InitVulkan();
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR(e.what());
    return false;
  }

  if (result)
  {
    return true;
  }

  LOG_CORE_ERROR("Failed to initialize Vulkan");
  return false;
}

//===============================================================================
void Renderer::Shutdown()
{
  m_VulkanRenderer.Shutdown();
  m_VulkanContext.Shutdown();
}

//===============================================================================
bool Renderer::InitVulkan()
{
  return m_VulkanContext.Init() && m_VulkanRenderer.Init();
}

} // namespace four
