#include "four-pch.h"

#include "window/glfw/glfwWindow.hpp"

#include "GLFW/glfw3.h"

namespace four
{
//----------------------------------------------------------------------------------------
GlfwWindow::GlfwWindow(std::string_view title, uint32_t width, uint32_t height) :
m_Window(nullptr),
m_Title(title),
m_Width(width),
m_Height(height)
{
  glfwSetErrorCallback(GlfwErrorsCallback);

  if (glfwInit() == GLFW_FALSE)
  {
    return;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  m_Window = glfwCreateWindow(static_cast<int32_t>(m_Width), static_cast<int32_t>(m_Height), m_Title.data(), nullptr, nullptr);
  glfwSetWindowUserPointer(m_Window, this);
  glfwSetFramebufferSizeCallback(m_Window, FrameBufferResizedCallback);

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
void GlfwWindow::FrameBufferResizedCallback(GLFWwindow* window, int32_t width, int32_t height)
{
  auto self                  = reinterpret_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
  self->m_FrameBufferResized = true;
  self->m_Width              = static_cast<uint32_t>(width);
  self->m_Height             = static_cast<uint32_t>(height);
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

void GlfwWindow::WaitEventsImpl() const
{
  glfwWaitEvents();
}


} //namespace four
