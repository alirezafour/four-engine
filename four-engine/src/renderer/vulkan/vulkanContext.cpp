#include "four-pch.h"

#include "renderer/vulkan/vulkanContext.hpp"

#include "window/glfw/glfwWindow.hpp"
#include "GLFW/glfw3.h"

namespace four
{

//===============================================================================
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::DebugMessengerCallBack(
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
VulkanContext::VulkanContext(Window<GlfwWindow>& window) : m_Window{window}
{
}

//===============================================================================
bool VulkanContext::Init()
{
  return CreateInstance() && SetupDebugMessenger() && CreateSurface() && PickPhysicalDevice();
}

//===============================================================================
void VulkanContext::Shutdown()
{
  if (m_Instance)
  {
    m_Device.destroy();
    m_Instance.destroySurfaceKHR(m_Surface);
    m_Instance.destroy();
  }
}

//===============================================================================
bool VulkanContext::CreateInstance()
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
bool VulkanContext::SetupDebugMessenger()
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
bool VulkanContext::CreateSurface()
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
bool VulkanContext::PickPhysicalDevice()
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
      // break;
    }
  }

  // if could not find a suitable GPU
  if (m_PhysicalDevice == VK_NULL_HANDLE)
  {
    LOG_CORE_ERROR("failed to find a suitable GPU!");
    return false;
  }

  // TODO: move this part later // pick swapchain extent
  const auto capabilities = m_PhysicalDevice.getSurfaceCapabilitiesKHR(m_Surface);
  m_SwapChainExtent       = ChooseSwapExtent(capabilities);

  return true;
}

//===============================================================================
bool VulkanContext::CreateLogicalDevice()
{
  const auto indices = FindQueueFamilies(m_PhysicalDevice, m_Surface);

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
bool VulkanContext::CheckValidationLayerSupport()
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
std::vector<const char*> VulkanContext::GetRequiredExtensions()
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
bool VulkanContext::IsDeviceSuitable(const vk::PhysicalDevice& device) const
{
  const auto indices             = FindQueueFamilies(device, m_Surface);
  const bool extensionsSupported = CheckDeviceExtensionSupport(device);
  bool       swapChainAquate     = false;
  if (extensionsSupported)
  {
    const auto swapChainSupport = QuerySwapChainSupport(device, m_Surface);
    swapChainAquate             = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }

  return indices.IsComplete() && extensionsSupported && swapChainAquate;
}

//===============================================================================
bool VulkanContext::CheckDeviceExtensionSupport(const vk::PhysicalDevice& device) const
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
VulkanContext::SwapChainSupportDetails VulkanContext::QuerySwapChainSupport(const vk::PhysicalDevice& device,
                                                                            const vk::SurfaceKHR&     surface)
{
  return {.capabilities = device.getSurfaceCapabilitiesKHR(surface),
          .formats      = device.getSurfaceFormatsKHR(surface),
          .presentModes = device.getSurfacePresentModesKHR(surface)};
}

//===============================================================================
VulkanContext::QueueFamilyIndices VulkanContext::FindQueueFamilies(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface)
{
  // logic to find queue families
  auto queueFamilies = device.getQueueFamilyProperties();

  QueueFamilyIndices indices;
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
uint32_t VulkanContext::RateDeviceSuitability(const vk::PhysicalDevice& device) const
{
  // TODO: temporary scoring
  uint32_t score = 0;

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
void VulkanContext::PrintExtensionsSupport()
{
  LOG_CORE_INFO("available extensions:");
  const std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();
  for (const auto& extension : availableExtensions)
  {
    LOG_CORE_INFO("\tExtension: {}", extension.extensionName);
  }
}

//===============================================================================
vk::Extent2D VulkanContext::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
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
