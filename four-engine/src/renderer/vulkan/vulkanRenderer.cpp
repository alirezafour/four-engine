#include "four-pch.h"

#include "renderer/vulkan/vulkanRenderer.hpp"
#include "window/glfw/glfwWindow.hpp"

namespace four
{

//===============================================================================
VulkanRenderer::VulkanRenderer(Window<GlfwWindow>& window, VulkanContext& context) :
m_Window{window},
m_VulkanContext{context}
{
}

//===============================================================================
VulkanRenderer::~VulkanRenderer() = default;

//===============================================================================
bool VulkanRenderer::Init()
{
  return CreateSwapChain() && CreateImageViews();
}

//===============================================================================
void VulkanRenderer::Shutdown()
{
  const auto device = m_VulkanContext.GetDevice();
  for (auto imageView : m_SwapChainImageViews)
  {
    device.destroyImageView(imageView);
  }
  device.destroySwapchainKHR(m_SwapChain);
}


//===============================================================================
bool VulkanRenderer::CreateImageViews()
{
  m_SwapChainImageViews.resize(m_SwapChainImages.size());

  for (size_t i = 0; i < m_SwapChainImages.size(); ++i)
  {
    vk::ImageViewCreateInfo createInfo{};
    createInfo.sType = vk::StructureType::eImageViewCreateInfo;
    createInfo.image = m_SwapChainImages[i]; // swapchain image.

    createInfo.viewType = vk::ImageViewType::e2D;
    createInfo.format   = m_SwapChainImageFormat;

    createInfo.components.r = vk::ComponentSwizzle::eIdentity;
    createInfo.components.g = vk::ComponentSwizzle::eIdentity;
    createInfo.components.b = vk::ComponentSwizzle::eIdentity;
    createInfo.components.a = vk::ComponentSwizzle::eIdentity;

    createInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    createInfo.subresourceRange.baseMipLevel   = 0;
    createInfo.subresourceRange.levelCount     = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount     = 1;

    const auto device        = m_VulkanContext.GetDevice();
    m_SwapChainImageViews[i] = device.createImageView(createInfo);
    if (!m_SwapChainImageViews[i])
    {
      LOG_CORE_ERROR("failed to create image views!");
      return false;
    }
  }

  return true;
}

//===============================================================================
bool VulkanRenderer::CreateSwapChain()
{
  const auto physicalDevice   = m_VulkanContext.GetPhysicalDevice();
  const auto surface          = m_VulkanContext.GetSurface();
  const auto device           = m_VulkanContext.GetDevice();
  const auto indices          = VulkanContext::FindQueueFamilies(physicalDevice, surface);
  const auto swapchainSupport = VulkanContext::QuerySwapChainSupport(physicalDevice, surface);
  const auto extent           = m_VulkanContext.GetExtent();

  auto surfaceFormat = ChooseSwapSurfaceFormat(swapchainSupport.formats);
  auto presentMode   = ChooseSwapPresentMode(swapchainSupport.presentModes);

  uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
  if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
  {
    imageCount = swapchainSupport.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR createInfo{};
  createInfo.sType            = vk::StructureType::eSwapchainCreateInfoKHR;
  createInfo.surface          = surface;
  createInfo.minImageCount    = imageCount;
  createInfo.imageFormat      = surfaceFormat.format;
  createInfo.imageColorSpace  = surfaceFormat.colorSpace;
  createInfo.imageExtent      = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;

  std::array<uint32_t, 2> queueFamilyIndices = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily)
  {
    createInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
  }
  else
  {
    createInfo.imageSharingMode      = vk::SharingMode::eExclusive;
    createInfo.queueFamilyIndexCount = 0;       // Optional
    createInfo.pQueueFamilyIndices   = nullptr; // Optional
  }

  createInfo.preTransform   = swapchainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  createInfo.presentMode    = presentMode;
  createInfo.clipped        = VK_TRUE;

  createInfo.oldSwapchain = nullptr;

  m_SwapChain = device.createSwapchainKHR(createInfo);
  if (!m_SwapChain)
  {
    LOG_CORE_ERROR("failed to create swap chain!");
    return false;
  }

  m_SwapChainImages      = device.getSwapchainImagesKHR(m_SwapChain);
  m_SwapChainImageFormat = surfaceFormat.format;
  m_SwapChainExtent      = extent;

  return true;
}

//===============================================================================
vk::SurfaceFormatKHR VulkanRenderer::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{

  assert(!availableFormats.empty() && "No available formats found");

  for (const auto& availableFormat : availableFormats)
  {
    if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
        availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
    {
      return availableFormat;
    }
  }
  //INFO: we could rank them how good they are, but for now just pick the first one
  return availableFormats[0];
}

//===============================================================================
vk::PresentModeKHR VulkanRenderer::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
  for (const auto& availablePresentMode : availablePresentModes)
  {
    if (availablePresentMode == vk::PresentModeKHR::eMailbox)
    {
      return availablePresentMode;
    }
  }
  return vk::PresentModeKHR::eFifo;
}

} // namespace four
