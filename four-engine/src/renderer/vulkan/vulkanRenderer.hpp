#pragma once

#include "core/core.hpp"
#include "renderer/vulkan/vulkanContext.hpp"

namespace four
{

class GlfwWindow;
template <typename T>
class Window;

class FOUR_ENGINE_API VulkanRenderer
{
public:
  explicit VulkanRenderer(Window<GlfwWindow>& window, VulkanContext& context);
  ~VulkanRenderer();
  VulkanRenderer(const VulkanRenderer&)                            = delete;
  VulkanRenderer(VulkanRenderer&&)                                 = delete;
  VulkanRenderer&                 operator=(const VulkanRenderer&) = delete;
  VulkanRenderer&                 operator=(VulkanRenderer&&)      = delete;
  [[nodiscard]] inline vk::Format GetSwapChainImageFormat() const
  {
    return m_SwapChainImageFormat;
  }


private:
  [[nodiscard]] bool Init();
  void               Shutdown();

  [[nodiscard]] bool CreateSwapChain();
  [[nodiscard]] bool CreateImageViews();

  [[nodiscard]] static vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
  [[nodiscard]] static vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

private:
  Window<GlfwWindow>& m_Window;
  VulkanContext&      m_VulkanContext;

  vk::SwapchainKHR           m_SwapChain;
  std::vector<vk::Image>     m_SwapChainImages;
  vk::Format                 m_SwapChainImageFormat;
  vk::Extent2D               m_SwapChainExtent;
  std::vector<vk::ImageView> m_SwapChainImageViews;
};

} // namespace four
