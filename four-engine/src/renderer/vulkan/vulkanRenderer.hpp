#pragma once

#include "core/core.hpp"

#include "vulkan/vulkan.hpp"

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
  VulkanRenderer(const VulkanRenderer&)            = delete;
  VulkanRenderer(VulkanRenderer&&)                 = delete;
  VulkanRenderer& operator=(const VulkanRenderer&) = delete;
  VulkanRenderer& operator=(VulkanRenderer&&)      = delete;

  [[nodiscard]] bool Init();
  void               Shutdown();

  [[nodiscard]] const vk::Device& GetDevice() const
  {
    return m_Device;
  }

  [[nodiscard]] vk::Extent2D GetExtent() const
  {
    return m_SwapChainExtent;
  }


private:
  [[nodiscard]] bool CreateLogicalDevice();
  [[nodiscard]] bool CreateSwapChain();
  [[nodiscard]] bool CreateImageViews();

  [[nodiscard]] VulkanContext::QueueFamilyIndices      FindQueueFamilies() const;
  [[nodiscard]] VulkanContext::SwapChainSupportDetails QuerySwapChainSupport() const;

  [[nodiscard]] static vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
  [[nodiscard]] static vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
  [[nodiscard]] vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

private:
  Window<GlfwWindow>& m_Window;
  VulkanContext&      m_VulkanContext;
  vk::Device          m_Device;
  vk::Queue           m_GraphicsQueue;
  vk::Queue           m_PresentQueue;

  vk::SwapchainKHR           m_SwapChain;
  std::vector<vk::Image>     m_SwapChainImages;
  vk::Format                 m_SwapChainImageFormat;
  vk::Extent2D               m_SwapChainExtent;
  std::vector<vk::ImageView> m_SwapChainImageViews;
};

} // namespace four
