#include "four-pch.h"

#include "renderer/vulkan/vulkanRenderer.hpp"
#include "window/glfw/glfwWindow.hpp"

namespace four
{

//===============================================================================
VulkanRenderer::VulkanRenderer(Window<GlfwWindow>& window, VulkanContext& context) :
m_Window(window),
m_VulkanContext(context)
{
}

//===============================================================================
VulkanRenderer::~VulkanRenderer() = default;

//===============================================================================
bool VulkanRenderer::Init()
{
  return CreateLogicalDevice() && CreateSwapChain() && CreateImageViews();
}

//===============================================================================
void VulkanRenderer::Shutdown()
{
  for (auto imageView : m_SwapChainImageViews)
  {
    m_Device.destroyImageView(imageView);
  }
  m_Device.destroySwapchainKHR(m_SwapChain);
  m_Device.destroy();
}

//===============================================================================
bool VulkanRenderer::CreateLogicalDevice()
{
  const auto physicalDevice   = m_VulkanContext.GetPhysicalDevice();
  const auto deviceExtensions = m_VulkanContext.GetDeviceExtensions();
  const auto indices          = FindQueueFamilies();

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
  for (float queuePriority = 1.f; const auto& queueFamily : uniqueQueueFamilies)
  {
    vk::DeviceQueueCreateInfo queueInfo{};
    queueInfo.sType            = vk::StructureType::eDeviceQueueCreateInfo;
    queueInfo.queueFamilyIndex = queueFamily;
    queueInfo.queueCount       = 1;
    queueInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueInfo);
  }
  auto features = physicalDevice.getFeatures();

  vk::DeviceCreateInfo createInfo{};
  createInfo.sType                   = vk::StructureType::eDeviceCreateInfo;
  createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos       = queueCreateInfos.data();
  createInfo.pEnabledFeatures        = &features;
  createInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  m_Device = physicalDevice.createDevice(createInfo);
  if (!m_Device)
  {
    LOG_CORE_ERROR("failed to create logical device!");
    return false;
  }

  // get handle to graphics queue that created by the device
  m_GraphicsQueue = m_Device.getQueue(indices.graphicsFamily.value(), 0);

  // get handle to present queue that created by the device
  m_PresentQueue = m_Device.getQueue(indices.presentFamily.value(), 0);

  return true;
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

    m_SwapChainImageViews[i] = m_Device.createImageView(createInfo);
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
  const auto physicalDevice = m_VulkanContext.GetPhysicalDevice();
  const auto surface        = m_VulkanContext.GetSurface();
  const auto indices        = FindQueueFamilies();

  auto swapchainSupport = QuerySwapChainSupport();
  auto surfaceFormat    = ChooseSwapSurfaceFormat(swapchainSupport.formats);
  auto presentMode      = ChooseSwapPresentMode(swapchainSupport.presentModes);
  auto extent           = ChooseSwapExtent(swapchainSupport.capabilities);

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

  m_SwapChain = m_Device.createSwapchainKHR(createInfo);
  if (!m_SwapChain)
  {
    LOG_CORE_ERROR("failed to create swap chain!");
    return false;
  }

  m_SwapChainImages      = m_Device.getSwapchainImagesKHR(m_SwapChain);
  m_SwapChainImageFormat = surfaceFormat.format;
  m_SwapChainExtent      = extent;

  return true;
}

//===============================================================================
VulkanContext::SwapChainSupportDetails VulkanRenderer::QuerySwapChainSupport() const
{
  const auto device  = m_VulkanContext.GetPhysicalDevice();
  const auto surface = m_VulkanContext.GetSurface();

  return {.capabilities = device.getSurfaceCapabilitiesKHR(surface),
          .formats      = device.getSurfaceFormatsKHR(surface),
          .presentModes = device.getSurfacePresentModesKHR(surface)};
}

//===============================================================================
VulkanContext::QueueFamilyIndices VulkanRenderer::FindQueueFamilies() const
{
  const auto device  = m_VulkanContext.GetPhysicalDevice();
  const auto surface = m_VulkanContext.GetSurface();

  // logic to find queue families
  auto queueFamilies = device.getQueueFamilyProperties();

  VulkanContext::QueueFamilyIndices indices;
  for (uint32_t index = 0u; const auto& queueFamily : queueFamilies)
  {
    if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
    {
      indices.graphicsFamily = index;
      if (device.getSurfaceSupportKHR(index, surface) != 0u)
      {
        indices.presentFamily = index;
      }

      if (indices.IsComplete())
      {
        break;
      }
    }
    ++index;
  }
  return indices;
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

//===============================================================================
vk::Extent2D VulkanRenderer::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
  {
    return capabilities.currentExtent;
  }
  const uint32_t width  = m_Window.GetExtent().GetWidth();
  const uint32_t height = m_Window.GetExtent().GetHeight();
  vk::Extent2D   actualExtent{width, height};

  actualExtent.width  = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
  actualExtent.height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

  return actualExtent;
}
} // namespace four
