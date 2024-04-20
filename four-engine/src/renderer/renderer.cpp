#include "four-pch.h"

#include "renderer/renderer.hpp"

#include "window/glfw/glfwWindow.hpp"
#include "GLFW/glfw3.h"

namespace four
{

//===============================================================================
Renderer::Renderer(Window<GlfwWindow>& window) : m_Window{window}
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
  if (m_Instance)
  {
    for (auto imageView : m_SwapChainImageViews)
    {
      m_Device.destroyImageView(imageView);
    }
    m_Device.destroySwapchainKHR(m_SwapChain);
    m_Device.destroy();
    m_Instance.destroySurfaceKHR(m_Surface);
    m_Instance.destroy();
  }
}

//===============================================================================
bool Renderer::InitVulkan()
{
  return CreateVulkanInstance() && SetupDebugMessenger() && CreateSurface() && PickPhysicalDevice() &&
         CreateLogicalDevice() && CreateSwapChain() && CreateImageView();
}

//===============================================================================
bool Renderer::CreateVulkanInstance()
{
  if (EnableValidationLayers && !CheckValidationLayerSupport())
  {
    LOG_CORE_ERROR("validation layers requested, but not available!");
    return false;
  }

  vk::ApplicationInfo    appInfo{"four-engine", 1, "four-engine", 1, VK_API_VERSION_1_3};
  vk::InstanceCreateInfo createInfo{vk::InstanceCreateFlags(), &appInfo};

  auto extensions = GetRequiredExtensions();

  createInfo.enabledExtensionCount   = extensions.size();
  createInfo.ppEnabledExtensionNames = extensions.data();

  if (EnableValidationLayers)
  {
    createInfo.enabledLayerCount   = ValidationLayers.size();
    createInfo.ppEnabledLayerNames = ValidationLayers.data();
  }
  else
  {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext             = nullptr;
  }

  m_Instance = vk::createInstance(createInfo);
  if (!m_Instance)
  {
    LOG_CORE_ERROR("failed to create instance!");
    return false;
  }
  return true;
}

//===============================================================================
bool Renderer::SetupDebugMessenger()
{
  if (!EnableValidationLayers)
  {
    return true;
  }

  vk::DebugUtilsMessengerCreateInfoEXT createInfo;
  createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
  createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
  createInfo.pfnUserCallback = DebugMessengerCallBack;

  // TODO: unable to fix it yet ( linker error )
  // m_DebugMessenger           = m_Instance.createDebugUtilsMessengerEXT(createInfo, nullptr);
  // if (!m_DebugMessenger)
  // {
  //   throw std::runtime_error("failed to set up debug messenger!");
  // }
  return true;
}

//===============================================================================
bool Renderer::PickPhysicalDevice()
{
  m_PhysicalDevice     = VK_NULL_HANDLE;
  auto physicalDevices = m_Instance.enumeratePhysicalDevices();
  if (physicalDevices.empty())
  {
    LOG_CORE_ERROR("failed to find GPUs with Vulkan support!");
    return false;
  }

  for (const auto& device : physicalDevices)
  {
    if (IsDeviceSuitable(device))
    {
      m_PhysicalDevice = device;
    }
  }

  // if could not find a suitable GPU
  if (m_PhysicalDevice == VK_NULL_HANDLE)
  {
    LOG_CORE_ERROR("failed to find a suitable GPU!");
    return false;
  }
  return true;
}

//===============================================================================
bool Renderer::CreateLogicalDevice()
{
  auto                                   indices = FindQueueFamilies(m_PhysicalDevice);
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
  auto features = m_PhysicalDevice.getFeatures();

  vk::DeviceCreateInfo createInfo{};
  createInfo.sType                   = vk::StructureType::eDeviceCreateInfo;
  createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos       = queueCreateInfos.data();
  createInfo.pEnabledFeatures        = &features;
  createInfo.enabledExtensionCount   = static_cast<uint32_t>(m_DeviceExtensions.size());
  createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

  m_Device = m_PhysicalDevice.createDevice(createInfo);
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
bool Renderer::CreateSurface()
{
  auto* window = m_Window.GetHandle();
  assert(window != nullptr && "Window handle is null");

  VkSurfaceKHR surface = VK_NULL_HANDLE;
  if (glfwCreateWindowSurface(m_Instance, window, nullptr, &surface) != VK_SUCCESS)
  {
    LOG_CORE_ERROR("failed to create window surface!");
    return false;
  }
  m_Surface = vk::SurfaceKHR(surface);
  if (m_Surface == VK_NULL_HANDLE)
  {
    LOG_CORE_ERROR("failed to create window surface!");
    return false;
  }
  return true;
}

//===============================================================================
bool Renderer::CreateSwapChain()
{
  auto swapchainSupport = QuerySwapChainSupport(m_PhysicalDevice);
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
  createInfo.surface          = m_Surface;
  createInfo.minImageCount    = imageCount;
  createInfo.imageFormat      = surfaceFormat.format;
  createInfo.imageColorSpace  = surfaceFormat.colorSpace;
  createInfo.imageExtent      = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;

  auto                    indices            = FindQueueFamilies(m_PhysicalDevice);
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
bool Renderer::CreateImageView()
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
void Renderer::PrintExtensionsSupport()
{
  LOG_CORE_INFO("available extensions:");
  const std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();
  for (const auto& extension : availableExtensions)
  {
    LOG_CORE_INFO("\tExtension: {}", extension.extensionName);
  }
}

//===============================================================================
bool Renderer::CheckValidationLayerSupport()
{
  std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
  for (const char* layerName : ValidationLayers)
  {
    bool layerFound = false;
    for (const auto& layerProperties : availableLayers)
    {
      if (strcmp(layerName, layerProperties.layerName) == 0)
      {
        layerFound = true;
        break;
      }
    }
    if (!layerFound)
    {
      return false;
    }
  }
  return true;
}

//===============================================================================
std::vector<const char*> Renderer::GetRequiredExtensions() const
{
  // add window extension enable
  std::vector<const char*> extensions = m_Window.GetVulkanRequiredExtensions();

  // check for vaidation layer
  if (EnableValidationLayers)
  {
    extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  return extensions;
}

//===============================================================================
VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::DebugMessengerCallBack(
  VkDebugUtilsMessageSeverityFlagBitsEXT      messageSensivity,
  VkDebugUtilsMessageTypeFlagsEXT             messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
  void* /*pUserData*/)
{
  if (messageSensivity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
  {
    LOG_CORE_ERROR("validation layer: {}", pCallbackData->pMessage);
  }
  return VK_FALSE;
}

//===============================================================================
bool Renderer::IsDeviceSuitable(const vk::PhysicalDevice& device) const
{
  const auto indices             = FindQueueFamilies(device);
  const bool extensionsSupported = CheckDeviceExtensionSupport(device);
  bool       swapChainAquate     = false;
  if (extensionsSupported)
  {
    const auto swapChainSupport = QuerySwapChainSupport(device);
    swapChainAquate             = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }

  return indices.IsComplete() && extensionsSupported && swapChainAquate;
}

//===============================================================================
bool Renderer::CheckDeviceExtensionSupport(const vk::PhysicalDevice& device) const
{
  std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

  std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());
  for (const auto& extension : availableExtensions)
  {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

//===============================================================================
int Renderer::RateDeviceSuitability(const vk::PhysicalDevice& device) const
{
  // TODO: temporary scoring
  int score = 0;

  auto properties = device.getProperties();
  auto features   = device.getFeatures();
  switch (properties.deviceType)
  {
    case vk::PhysicalDeviceType::eDiscreteGpu:
      score += 1000;
      break;
    default:
      break;
  }

  score += properties.limits.maxImageDimension2D;
  if (features.geometryShader == 0u)
  {
    return 0;
  }
  return score;
}

//===============================================================================
Renderer::QueueFamilyIndices Renderer::FindQueueFamilies(const vk::PhysicalDevice& device) const
{
  // logic to find queue families
  auto queueFamilies = device.getQueueFamilyProperties();

  QueueFamilyIndices indices;
  for (uint32_t index = 0u; const auto& queueFamily : queueFamilies)
  {
    if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
    {
      indices.graphicsFamily = index;
      if (device.getSurfaceSupportKHR(index, m_Surface) != 0u)
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
Renderer::SwapChainSupportDetails Renderer::QuerySwapChainSupport(const vk::PhysicalDevice& device) const
{
  return {.capabilities = device.getSurfaceCapabilitiesKHR(m_Surface),
          .formats      = device.getSurfaceFormatsKHR(m_Surface),
          .presentModes = device.getSurfacePresentModesKHR(m_Surface)};
}


//===============================================================================
vk::SurfaceFormatKHR Renderer::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
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
vk::PresentModeKHR Renderer::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
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
vk::Extent2D Renderer::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
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
