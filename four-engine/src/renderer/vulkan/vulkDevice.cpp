#include "four-pch.h"

#include "renderer/vulkan/vulkDevice.hpp"
#include "window/glfw/glfwWindow.hpp"
#include "GLFW/glfw3.h"
#include "window/window.hpp"

namespace four
{

//===========================================================================================
static VkBool32 DebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                            VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
                                            VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
                                            void* /*pUserData*/)
{
#if !defined(NDEBUG)
  if (static_cast<uint32_t>(pCallbackData->messageIdNumber) == 0x822806fa)
  {
    // Validation Warning: vkCreateInstance(): to enable extension VK_EXT_debug_utils, but this extension is intended to support use by applications when
    // debugging and it is strongly recommended that it be otherwise avoided.
    return vk::False;
  }
  else if (static_cast<uint32_t>(pCallbackData->messageIdNumber) == 0xe8d1a9fe)
  {
    // Validation Performance Warning: Using debug builds of the validation layers *will* adversely affect performance.
    return vk::False;
  }
#endif

  LOG_INFO("{}: {}:\n\tmessageIdName       = <{}>\n\tmessageIdNumber        = {}\n\tmessage        = {}",
           vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)),
           vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)),
           pCallbackData->pMessageIdName,
           pCallbackData->messageIdNumber,
           pCallbackData->pMessage);
  if (0 < pCallbackData->queueLabelCount)
  {
    LOG_INFO("Queue Labels:");
    for (uint32_t i = 0; i < pCallbackData->queueLabelCount; ++i)
    {
      LOG_INFO("\tlabelName = <{}>", pCallbackData->pQueueLabels[i].pLabelName);
    }
  }
  if (0 < pCallbackData->cmdBufLabelCount)
  {
    LOG_INFO("CommandBuffer Labels:");
    for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; ++i)
    {
      LOG_INFO("\tlabelName = <{}>", pCallbackData->pCmdBufLabels[i].pLabelName);
    }
  }
  if (0 < pCallbackData->objectCount)
  {
    LOG_INFO("Objects:");
    for (uint32_t i = 0; i < pCallbackData->objectCount; ++i)
    {
      LOG_INFO("\tObject {}:", std::to_string(i));
      LOG_INFO("\t\tobjectType   = {}", vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)));
      LOG_INFO("\t\tobjectHandle = {}", std::to_string(pCallbackData->pObjects[i].objectHandle));
      if (pCallbackData->pObjects[i].pObjectName)
      {
        LOG_INFO("\t\tobjectName   = <{}>", pCallbackData->pObjects[i].pObjectName);
      }
    }
  }
  return vk::False;
}

//===========================================================================================
static vk::DebugUtilsMessengerCreateInfoEXT CreateDebugMessageInfoEXT()
{
  return {{},
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
          vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
          &DebugUtilsMessengerCallback};
}

//===========================================================================================
VulkDevice::VulkDevice(Window<GlfwWindow>* window) : m_Window(window)
{
}

//===========================================================================================
void VulkDevice::InitVulkan()
{
  CreateInstance();
  SetupDebugMessenger();
  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();
  CreateCommandPool();
}

//===========================================================================================
void VulkDevice::Cleanup()
{
  m_Device.destroyCommandPool(m_CommandPool);
  m_Device.destroy();
  m_Instance.destroySurfaceKHR(m_Surface);
  if (enableValidationLayers)
  {
    // m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMessenger);
  }
  m_Instance.destroy();
}

//===========================================================================================
uint32_t VulkDevice::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
  vk::PhysicalDeviceMemoryProperties memProperties = m_PhysicalDevice.getMemoryProperties();
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
  {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
    {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

//===========================================================================================
vk::Format VulkDevice::FindSupportedFormat(const std::vector<vk::Format>& candidates,
                                           vk::ImageTiling                tiling,
                                           vk::FormatFeatureFlags         features)
{
  for (auto format : candidates)
  {
    VkFormatProperties props = m_PhysicalDevice.getFormatProperties(format);
    if (tiling == vk::ImageTiling::eLinear && props.linearTilingFeatures & features == features)
    {
      return format;
    }
    else if (tiling == vk::ImageTiling::eOptimal && props.optimalTilingFeatures & features == features)
    {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}

//===========================================================================================
void VulkDevice::CreateBuffer(vk::DeviceSize          size,
                              vk::BufferUsageFlags    usage,
                              vk::MemoryPropertyFlags properties,
                              vk::Buffer&             buffer,
                              vk::DeviceMemory&       bufferMemory)
{
  vk::BufferCreateInfo bufferInfo{};
  bufferInfo.sType       = vk::StructureType::eBufferCreateInfo;
  bufferInfo.size        = size;
  bufferInfo.usage       = usage;
  bufferInfo.sharingMode = vk::SharingMode::eExclusive;

  buffer = m_Device.createBuffer(bufferInfo, nullptr);
  // if (!(m_Device.createBuffer(bufferInfo, nullptr, &buffer)))
  // {
  //   throw std::runtime_error("failed to create vertex buffer!");
  // }

  vk::MemoryRequirements memRequirements = m_Device.getBufferMemoryRequirements(buffer);

  vk::MemoryAllocateInfo allocInfo{};
  allocInfo.sType           = vk::StructureType::eMemoryAllocateInfo;
  allocInfo.allocationSize  = memRequirements.size;
  allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);


  bufferMemory = m_Device.allocateMemory(allocInfo);
  m_Device.bindBufferMemory(buffer, bufferMemory, 0);
}

//===========================================================================================
vk::CommandBuffer VulkDevice::BeginSingleTimeCommands()
{
  vk::CommandBufferAllocateInfo allocInfo{};
  allocInfo.sType              = vk::StructureType::eCommandBufferAllocateInfo;
  allocInfo.level              = vk::CommandBufferLevel::ePrimary;
  allocInfo.commandPool        = m_CommandPool;
  allocInfo.commandBufferCount = 1;

  vk::CommandBuffer commandBuffer = m_Device.allocateCommandBuffers(allocInfo).front();

  vk::CommandBufferBeginInfo beginInfo{};
  beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
  beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

  commandBuffer.begin(beginInfo);
  return commandBuffer;
}

//===========================================================================================
void VulkDevice::EndSingleTimeCommands(vk::CommandBuffer commandBuffer)
{
  commandBuffer.end();

  vk::SubmitInfo submitInfo{};
  submitInfo.sType              = vk::StructureType::eSubmitInfo;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &commandBuffer;

  m_GraphicsQueue.submit(submitInfo, VK_NULL_HANDLE);
  m_GraphicsQueue.waitIdle();

  m_Device.freeCommandBuffers(m_CommandPool, 1, &commandBuffer);
}

//===========================================================================================
void VulkDevice::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
  vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

  vk::BufferCopy copyRegion{};
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size      = size;
  commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

  EndSingleTimeCommands(commandBuffer);
}

//===========================================================================================
void VulkDevice::CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount)
{
  vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

  vk::BufferImageCopy region{};
  region.bufferOffset      = 0;
  region.bufferRowLength   = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
  region.imageSubresource.mipLevel       = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount     = layerCount;

  region.imageOffset = vk::Offset3D(0, 0, 0);
  region.imageExtent = vk::Extent3D(width, height, 1);

  commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
  EndSingleTimeCommands(commandBuffer);
}

//===========================================================================================
void VulkDevice::CreateImageWithInfo(const vk::ImageCreateInfo& imageInfo,
                                     vk::MemoryPropertyFlags    properties,
                                     vk::Image&                 image,
                                     vk::DeviceMemory&          imageMemory)
{

  image = m_Device.createImage(imageInfo, nullptr);

  vk::MemoryRequirements memRequirements = m_Device.getImageMemoryRequirements(image);
  uint32_t               memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);
  imageMemory = m_Device.allocateMemory(vk::MemoryAllocateInfo(memRequirements.size, memoryTypeIndex));

  m_Device.bindImageMemory(image, imageMemory, 0);
}

//===========================================================================================
void VulkDevice::CreateInstance()
{
  if (enableValidationLayers && !CheckValidationLayerSupport())
  {
    throw std::runtime_error("validation layers requested, but not available!");
  }


  vk::ApplicationInfo appInfo{"Hello Vulkan", 1, "No Engine", 1, VK_API_VERSION_1_1};

  vk::InstanceCreateInfo createInfo{{}, &appInfo};
  if (enableValidationLayers)
  {
    createInfo.enabledLayerCount   = static_cast<uint32_t>(m_ValidationLayers.size());
    createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
  }
  else
  {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext             = nullptr;
  }

  auto extentions                    = GetRequiredExtensions();
  createInfo.enabledExtensionCount   = static_cast<uint32_t>(extentions.size());
  createInfo.ppEnabledExtensionNames = extentions.data();


  m_Instance = vk::createInstance(createInfo);
  // HasGflwRequiredInstanceExtensions();
}

//===========================================================================================

//===========================================================================================
void VulkDevice::SetupDebugMessenger()
{
  if (!enableValidationLayers)
  {
    return;
  }

  // m_DebugMessenger = m_Instance.createDebugUtilsMessengerEXT(CreateDebugMessageInfoEXT());
}

//===========================================================================================
std::vector<const char*> VulkDevice::GetRequiredExtensions()
{
  uint32_t     glfwExtensionCount = 0;
  const char** glfwExtensions     = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (enableValidationLayers)
  {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}
//===========================================================================================
bool VulkDevice::CheckValidationLayerSupport()
{
  std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
  for (const char* layerName : m_ValidationLayers)
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
//===========================================================================================
void VulkDevice::CreateSurface()
{
  auto*        window  = m_Window->GetWindow();
  VkSurfaceKHR surface = nullptr;
  if (glfwCreateWindowSurface(m_Instance, window, nullptr, &surface) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create window surface!");
  }
  m_Surface = vk::SurfaceKHR(surface);
}

//===========================================================================================
void VulkDevice::PickPhysicalDevice()
{
  std::vector<vk::PhysicalDevice> devices = m_Instance.enumeratePhysicalDevices();
  if (devices.empty())
  {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }
  LOG_CORE_INFO("Device count: {}", devices.size());
  vk::PhysicalDeviceProperties properties;

  // pick last device
  for (const auto& device : devices)
  {
    if (IsDeviceSuitable(device))
    {
      m_PhysicalDevice = device;
    }
  }

  if (m_PhysicalDevice == nullptr)
  {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  properties = m_PhysicalDevice.getProperties();
  LOG_CORE_INFO("physical device: {}", properties.deviceName);
}

//===========================================================================================
void VulkDevice::CreateLogicalDevice()
{
  QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t>                     uniqueQueueFamilies{indices.graphicsFamily, indices.presentFamily};

  float queuePriority = 1.0F;
  for (uint32_t queueFamily : uniqueQueueFamilies)
  {
    queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags(), queueFamily, 1, &queuePriority);
  }

  vk::PhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy          = vk::True;

  vk::DeviceCreateInfo createInfo{};
  createInfo.sType = vk::StructureType::eDeviceCreateInfo;

  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos    = queueCreateInfos.data();

  createInfo.pEnabledFeatures        = &deviceFeatures;
  createInfo.enabledExtensionCount   = static_cast<uint32_t>(m_DeviceExtensions.size());
  createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

  m_Device = m_PhysicalDevice.createDevice(createInfo, nullptr);
  // if (auto result = m_PhysicalDevice.createDevice(createInfo, nullptr, &m_Device); !result)
  // {
  //   throw std::runtime_error("failed to create logical device!");
  // }

  m_Device.getQueue(indices.graphicsFamily, 0, &m_GraphicsQueue);
  m_Device.getQueue(indices.presentFamily, 0, &m_PresentQueue);
}

//===========================================================================================
void VulkDevice::CreateCommandPool()
{
  QueueFamilyIndices queueFamilyIndices = FindPhysicalQueueFamilies();

  vk::CommandPoolCreateInfo poolInfo = {};
  poolInfo.sType                     = vk::StructureType::eCommandPoolCreateInfo;
  poolInfo.queueFamilyIndex          = queueFamilyIndices.graphicsFamily;
  poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
  m_CommandPool  = m_Device.createCommandPool(poolInfo, nullptr);

  // if (auto result = m_Device.createCommandPool(poolInfo, nullptr, &m_CommandPool); !result)
  // {
  //   throw std::runtime_error("failed to create command pool!");
  // }
}

//===========================================================================================
bool VulkDevice::IsDeviceSuitable(const vk::PhysicalDevice& device)
{
  auto indices = FindQueueFamilies(device);
  if (!indices.IsComplete())
  {
    return false;
  }

  if (!CheckDeviceExtensionSupport(device))
  {
    return false;
  };

  auto swapChainSupport = QuerySwapChainSupport(device);
  if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
  {
    return false;
  }

  if (vk::PhysicalDeviceFeatures supportedFeatures = device.getFeatures(); supportedFeatures.samplerAnisotropy == 0)
  {
    return false;
  }

  return true;
}

//===========================================================================================
QueueFamilyIndices VulkDevice::FindQueueFamilies(const vk::PhysicalDevice& device)
{
  QueueFamilyIndices indices;

  std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

  for (int index = 0; const auto& queueFamily : queueFamilies)
  {
    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
    {
      indices.graphicsFamily         = index;
      indices.graphicsFamilyHasValue = true;
    }

    vk::Bool32 presentSupport = 0U;
    vk::Result result         = device.getSurfaceSupportKHR(index, m_Surface, &presentSupport);
    if (result != vk::Result::eSuccess)
    {
      LOG_CORE_ERROR("failed to get surface support");
      throw std::runtime_error("failed to get surface support");
    }
    if (queueFamily.queueCount > 0 && (presentSupport == 0))
    {
      indices.presentFamily         = index;
      indices.presentFamilyHasValue = true;
    }
    if (indices.IsComplete())
    {
      break;
    }

    ++index;
  }

  return indices;
}

//===========================================================================================
void VulkDevice::HasGflwRequiredInstanceExtensions()
{
  std::vector<vk::ExtensionProperties> extensions = vk::enumerateInstanceExtensionProperties();
  LOG_CORE_INFO("available extensions:");
  std::unordered_set<std::string> available;
  for (const auto& extension : extensions)
  {
    LOG_CORE_INFO("\t{}", extension.extensionName);
    available.insert(extension.extensionName);
  }

  LOG_CORE_INFO("required extensions:");
  auto requiredExtensions = GetRequiredExtensions();
  for (const auto& required : requiredExtensions)
  {
    LOG_CORE_INFO("\t{}", required);
    if (available.find(required) == available.end())
    {
      throw std::runtime_error("Missing required glfw extension");
    }
  }
}

//===========================================================================================
bool VulkDevice::CheckDeviceExtensionSupport(const vk::PhysicalDevice& device)
{
  std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

  std::set<std::string> requiredExtensions{m_DeviceExtensions.begin(), m_DeviceExtensions.end()};

  for (const auto& extension : availableExtensions)
  {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

//===========================================================================================
SwapChainSupportDetails VulkDevice::QuerySwapChainSupport(const vk::PhysicalDevice& device)
{
  SwapChainSupportDetails details{};
  details.capabilities = device.getSurfaceCapabilitiesKHR(m_Surface);
  details.formats      = device.getSurfaceFormatsKHR(m_Surface);
  details.presentModes = device.getSurfacePresentModesKHR(m_Surface);

  return details;
}
} // namespace four
