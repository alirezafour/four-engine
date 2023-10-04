#include "four-pch.h"

#include "window/glfw/glfwWindow.hpp"

#include "GLFW/glfw3.h"

namespace four
{
//----------------------------------------------------------------------------------------
GlfwWindow::GlfwWindow(std::string_view title, uint32_t width, uint32_t height) :
m_Window(nullptr),
m_Width(width),
m_Height(height)
{
  glfwSetErrorCallback(GlfwWindow::GlfwErrorsCallback);

  if (glfwInit() == GLFW_FALSE)
  {
    return;
  }
  // setup default hint
  // @todo
  // glfwInitHint(GLFW_CLIENT_API, GLFW_NO_API);
  // glfwInitHint(GLFW_RESIZABLE, GLFW_FALSE);

  m_Window = glfwCreateWindow(width, height, std::string(title).data(), nullptr, nullptr);
  LOG_CORE_INFO("Init glfw Window");
}

//----------------------------------------------------------------------------------------
GlfwWindow::~GlfwWindow()
{
  Shutdown();
}

//----------------------------------------------------------------------------------------
void GlfwWindow::Shutdown()
{
  if (m_Window != nullptr)
  {
    LOG_CORE_INFO("Shutdown glfw Window");
    glfwDestroyWindow(m_Window);
    glfwTerminate();
    m_Window = nullptr;
  }
}

//----------------------------------------------------------------------------------------
bool GlfwWindow::ShouldClose() const noexcept
{
  return static_cast<bool>(glfwWindowShouldClose(m_Window));
}

//----------------------------------------------------------------------------------------
void GlfwWindow::OnUpdate()
{
  glfwPollEvents();
}

//----------------------------------------------------------------------------------------
uint32_t GlfwWindow::GetWidth() const noexcept
{
  return m_Width;
}

//----------------------------------------------------------------------------------------
uint32_t GlfwWindow::GetHeight() const noexcept
{
  return m_Height;
}

} //namespace four
