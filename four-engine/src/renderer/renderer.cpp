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
  try
  {
    InitVulkan();
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR(e.what());
    return false;
  }
  return true;
}

//===============================================================================
void Renderer::Shutdown()
{
  if (m_Instance)
  {
    m_Device.destroy();
    m_Instance.destroySurfaceKHR(m_Surface);
    m_Instance.destroy();
    LOG_CORE_INFO("Vulkan uninitialized");
  }
}

//===============================================================================
bool Renderer::InitVulkan()
{
  CreateVulkanInstance();
  SetupDebugMessenger();
  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();
  LOG_CORE_INFO("Vulkan initialized");
  return true;
}

//===============================================================================
void Renderer::CreateVulkanInstance()
{
  if (EnableValidationLayers && !CheckValidationLayerSupport())
  {
    throw std::runtime_error("validation layers requested, but not available!");
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
  LOG_CORE_INFO("check point");
  if (!m_Instance)
  {
    throw std::runtime_error("failed to create instance!");
  }

  LOG_CORE_INFO("Vulkan instance created");
}

//===============================================================================
void Renderer::SetupDebugMessenger()
{
  if (!EnableValidationLayers)
  {
    return;
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
  LOG_CORE_INFO("Vulkan debug messenger created");
}

//===============================================================================
void Renderer::PickPhysicalDevice()
{
  m_PhysicalDevice     = VK_NULL_HANDLE;
  auto physicalDevices = m_Instance.enumeratePhysicalDevices();
  if (physicalDevices.empty())
  {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
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
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  LOG_CORE_INFO("Vulkan physical device picked");
}

//===============================================================================
void Renderer::CreateLogicalDevice()
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
    throw std::runtime_error("failed to create logical device!");
  }

  // get handle to graphics queue that created by the device
  m_GraphicsQueue = m_Device.getQueue(indices.graphicsFamily.value(), 0);

  // get handle to present queue that created by the device
  m_PresentQueue = m_Device.getQueue(indices.presentFamily.value(), 0);

  LOG_CORE_INFO("Vulkan logical device created");
}

//===============================================================================
void Renderer::CreateSurface()
{
  auto* window = m_Window.GetHandle();
  assert(window != nullptr && "Window handle is null");

  VkSurfaceKHR surface = VK_NULL_HANDLE;
  if (glfwCreateWindowSurface(m_Instance, window, nullptr, &surface) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create window surface!");
  }
  m_Surface = vk::SurfaceKHR(surface);
  if (m_Surface == VK_NULL_HANDLE)
  {
    throw std::runtime_error("failed to create window surface!");
  }

  LOG_CORE_INFO("Vulkan surface created");
}

//===============================================================================
void Renderer::PrintExtensionsSupport()
{
  LOG_CORE_INFO("available extensions:");
  std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();
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
std::vector<const char*> Renderer::GetRequiredExtensions()
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
bool Renderer::IsDeviceSuitable(const vk::PhysicalDevice& device)
{
  auto indices             = FindQueueFamilies(device);
  bool extensionsSupported = CheckDeviceExtensionSupport(device);
  return indices.IsComplete() && extensionsSupported;
}

//===============================================================================
bool Renderer::CheckDeviceExtensionSupport(const vk::PhysicalDevice& device)
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
int Renderer::RateDeviceSuitability(const vk::PhysicalDevice& device)
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
Renderer::QueueFamilyIndices Renderer::FindQueueFamilies(const vk::PhysicalDevice& device)
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
} // namespace four
