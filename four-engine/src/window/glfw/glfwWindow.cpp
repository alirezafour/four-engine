#include "event/WindowEvent.hpp"
#include "event/event.hpp"
#include "four-pch.hpp"

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
  glfwSetKeyCallback(m_Window, &GlfwWindow::KeyCallback);

  LOG_CORE_INFO("Init glfw Window");
}

//----------------------------------------------------------------------------------------
GlfwWindow::~GlfwWindow()
{
  ShutdownImpl();
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
void GlfwWindow::KeyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
{
}

//----------------------------------------------------------------------------------------
std::vector<const char*> GlfwWindow::GetVulkanRequiredExtensionsImpl()
{
  uint32_t     glfwExtensionCount = 0;
  const char** glfwExtensions     = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  return {glfwExtensions, glfwExtensions + glfwExtensionCount};
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
  m_CurrentTime    = glfwGetTime();
  double deltaTime = m_CurrentTime - m_LastTime;
  if (deltaTime >= 1.0)
  {
    int framerate{std::max(1, static_cast<int>(m_Frames / deltaTime))};
    m_Title = std::to_string(framerate) + " FPS";
    glfwSetWindowTitle(m_Window, m_Title.data());
    m_Frames   = -1;
    m_LastTime = m_CurrentTime;
  }
  ++m_Frames;
  glfwPollEvents();
  if (const int state = glfwGetKey(m_Window, GLFW_KEY_W); state == GLFW_PRESS)
  {
    LOG_CORE_INFO("W");
    for (auto& event : m_KeyEvents)
    {
      event.Notify(1);
    }
  }
  if (const int state = glfwGetKey(m_Window, GLFW_KEY_S); state == GLFW_PRESS)
  {
    LOG_CORE_INFO("S");
    for (auto& event : m_KeyEvents)
    {
      event.Notify(2);
    }
  }
  if (const int state = glfwGetKey(m_Window, GLFW_KEY_A); state == GLFW_PRESS)
  {
    LOG_CORE_INFO("A");
    for (auto& event : m_KeyEvents)
    {
      event.Notify(3);
    }
  }
  if (const int state = glfwGetKey(m_Window, GLFW_KEY_D); state == GLFW_PRESS)
  {
    LOG_CORE_INFO("D");
    for (auto& event : m_KeyEvents)
    {
      event.Notify(4);
    }
  }
  if (const int state = glfwGetKey(m_Window, GLFW_KEY_Y); state == GLFW_PRESS)
  {
    LOG_CORE_INFO("Y");
    for (auto& event : m_KeyEvents)
    {
      event.Notify(5);
    }
  }
  if (const int state = glfwGetKey(m_Window, GLFW_KEY_Y); state == GLFW_RELEASE)
  {
    LOG_CORE_INFO("Y released");
    for (auto& event : m_KeyEvents)
    {
      event.Notify(6);
    }
  }
  static float mouseLastX{0.0F};
  static float mouseLastY{0.0F};
  double       getMouseX{0.0F};
  double       getMouseY{0.0F};
  glfwGetCursorPos(m_Window, &getMouseX, &getMouseY);
  if (mouseLastX != static_cast<float>(getMouseX) || mouseLastY != static_cast<float>(getMouseY))
  {
    for (auto& event : m_MouseEvents)
    {
      event.Notify(mouseLastX - static_cast<float>(getMouseX), mouseLastY - static_cast<float>(getMouseY));
    }
    mouseLastX = static_cast<float>(getMouseX);
    mouseLastY = static_cast<float>(getMouseY);
  }
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

//----------------------------------------------------------------------------------------
void GlfwWindow::WaitEventsImpl() const
{
  glfwWaitEvents();
}

//----------------------------------------------------------------------------------------
} //namespace four
