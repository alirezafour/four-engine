#pragma once

#include "core/core.hpp"

#include "renderer/vulkan/vulkanContext.hpp"
#include "renderer/vulkan/vulkanRenderer.hpp"

#include <vulkan/vulkan.hpp>

namespace four
{

class GlfwWindow;
template <typename T>
class Window;

class Renderer
{
public:
  explicit Renderer(Window<GlfwWindow>& window);
  ~Renderer();

  Renderer(const Renderer&)                = delete;
  Renderer& operator=(const Renderer&)     = delete;
  Renderer(Renderer&&) noexcept            = delete;
  Renderer& operator=(Renderer&&) noexcept = delete;

  bool Init();
  void Shutdown();

private:
  [[nodiscard]] bool InitVulkan();

private:
  Window<GlfwWindow>& m_Window;
  VulkanContext       m_VulkanContext;
  VulkanRenderer      m_VulkanRenderer;
};
} // namespace four
