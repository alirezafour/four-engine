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
  glfwSetErrorCallback(GlfwErrorsCallback);

  if (glfwInit() == GLFW_FALSE)
  {
    return;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  m_Window = glfwCreateWindow(width, height, std::string(title).data(), nullptr, nullptr);
  glfwSetWindowUserPointer(m_Window, this);

  LOG_CORE_INFO("Init glfw Window");
}

//----------------------------------------------------------------------------------------
GlfwWindow::~GlfwWindow()
{
  Shutdown();
}

//----------------------------------------------------------------------------------------
void GlfwWindow::GlfwErrorsCallback(int32_t /*error*/, const char* descripton)
{
  LOG_CORE_ERROR("GLFW Error: {}", descripton);
}

//----------------------------------------------------------------------------------------
void GlfwWindow::ShutdownImpl()
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
bool GlfwWindow::ShouldCloseImpl() const noexcept
{
  return static_cast<bool>(glfwWindowShouldClose(m_Window));
}

//----------------------------------------------------------------------------------------
void GlfwWindow::OnUpdateImpl()
{
  glfwPollEvents();
}

//----------------------------------------------------------------------------------------
uint32_t GlfwWindow::GetWidthImpl() const noexcept
{
  return m_Width;
}

//----------------------------------------------------------------------------------------
uint32_t GlfwWindow::GetHeightImpl() const noexcept
{
  return m_Height;
}

} //namespace four
