#include "four-pch.h"

#include "renderer/vulkan/vulkanContext.hpp"

#include "window/glfw/glfwWindow.hpp"
#include "GLFW/glfw3.h"

#include <fstream>

namespace four
{

//===============================================================================
std::vector<char> ReadFile(const std::string& filename)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open())
  {
    throw std::runtime_error("failed to open file!");
  }
  const size_t      fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), static_cast<long long>(fileSize));
  file.close();
  return buffer;
}
//===============================================================================
std::unordered_map<VkInstance, PFN_vkCreateDebugUtilsMessengerEXT>  CreateDebugUtilsMessengerEXTDispatchTable;
std::unordered_map<VkInstance, PFN_vkDestroyDebugUtilsMessengerEXT> DestroyDebugUtilsMessengerEXTDispatchTable;
std::unordered_map<VkInstance, PFN_vkSubmitDebugUtilsMessageEXT>    SubmitDebugUtilsMessageEXTDispatchTable;

void loadDebugUtilsCommands(VkInstance instance)
{
  PFN_vkVoidFunction temp_fp;

  temp_fp = vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (!temp_fp)
    throw "Failed to load vkCreateDebugUtilsMessengerEXT"; // check shouldn't be necessary (based on spec)
  CreateDebugUtilsMessengerEXTDispatchTable[instance] = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(temp_fp);

  temp_fp = vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (!temp_fp)
    throw "Failed to load vkDestroyDebugUtilsMessengerEXT"; // check shouldn't be necessary (based on spec)
  DestroyDebugUtilsMessengerEXTDispatchTable[instance] = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(temp_fp);

  temp_fp = vkGetInstanceProcAddr(instance, "vkSubmitDebugUtilsMessageEXT");
  if (!temp_fp)
    throw "Failed to load vkSubmitDebugUtilsMessageEXT"; // check shouldn't be necessary (based on spec)
  SubmitDebugUtilsMessageEXTDispatchTable[instance] = reinterpret_cast<PFN_vkSubmitDebugUtilsMessageEXT>(temp_fp);
}

void unloadDebugUtilsCommands(VkInstance instance)
{
  CreateDebugUtilsMessengerEXTDispatchTable.erase(instance);
  DestroyDebugUtilsMessengerEXTDispatchTable.erase(instance);
  SubmitDebugUtilsMessageEXTDispatchTable.erase(instance);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
  VkInstance                                instance,
  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
  const VkAllocationCallbacks*              pAllocator,
  VkDebugUtilsMessengerEXT*                 pMessenger)
{
  auto dispatched_cmd = CreateDebugUtilsMessengerEXTDispatchTable.at(instance);
  return dispatched_cmd(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance                   instance,
                                                           VkDebugUtilsMessengerEXT     messenger,
                                                           const VkAllocationCallbacks* pAllocator)
{
  auto dispatched_cmd = DestroyDebugUtilsMessengerEXTDispatchTable.at(instance);
  return dispatched_cmd(instance, messenger, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkSubmitDebugUtilsMessageEXT(
  VkInstance                                  instance,
  VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData)
{
  auto dispatched_cmd = SubmitDebugUtilsMessageEXTDispatchTable.at(instance);
  return dispatched_cmd(instance, messageSeverity, messageTypes, pCallbackData);
}

//===============================================================================
VulkanContext::VulkanContext(Window<GlfwWindow>& window) : m_Window{window}
{
  const bool result = Init();
  if (!result)
  {
    LOG_CORE_ERROR("Failed to initialize Vulkan context.");
  }
}

//===============================================================================
VulkanContext::~VulkanContext()
{
  Shutdown();
}

//===============================================================================
bool VulkanContext::Init()
{
  return CreateInstance() && SetupDebugMessenger() && CreateSurface() && PickPhysicalDevice() && CreateLogicalDevice() &&
         CreateSwapChain() && CreateImageViews() && CreateRenderPass() && CreateGraphicsPipeline() &&
         CreateFramebuffers() && CreateCommandPool() && CreateCommandBuffers() && CreateSyncObjects();
}

//===============================================================================
void VulkanContext::Shutdown()
{
  if (m_Instance)
  {
    m_Device.destroySemaphore(m_RenderFinishedSemaphore);
    m_Device.destroySemaphore(m_ImageAvailableSemaphore);
    m_Device.destroyFence(m_InFlightFence);
    m_Device.destroyCommandPool(m_CommandPool);
    for (auto framebuffer : m_SwapChainFramebuffers)
    {
      m_Device.destroyFramebuffer(framebuffer);
    }
    m_Device.destroyPipelineLayout(m_PipelineLayout);
    m_Device.destroyPipeline(m_GraphicsPipeline);
    m_Device.destroyRenderPass(m_RenderPass);
    m_Device.destroyShaderModule(m_FragmentShaderModule);
    m_Device.destroyShaderModule(m_VertexShaderModule);
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
  LOG_CORE_INFO("Device count: {}", physicalDevices.size());

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

  const auto properties = m_PhysicalDevice.getProperties();
  LOG_CORE_INFO("GPU: {}", properties.deviceName);

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
vk::Format VulkanContext::FindSupportedFormat(const std::vector<vk::Format>& candidates,
                                              vk::ImageTiling                tiling,
                                              vk::FormatFeatureFlags         features) const
{
  for (const auto& format : candidates)
  {
    const vk::FormatProperties props = m_PhysicalDevice.getFormatProperties(format);
    if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures == features))
    {
      return format;
    }
    else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures == features))
    {
      return format;
    }
  }

  LOG_CORE_ERROR("failed to find supported format!");
  return {};
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
vk::Extent2D VulkanContext::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
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

//===============================================================================
bool VulkanContext::CreateImageViews()
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

    m_SwapChainImageViews[i] = GetDevice().createImageView(createInfo);
    if (!m_SwapChainImageViews[i])
    {
      LOG_CORE_ERROR("failed to create image views!");
      return false;
    }
  }

  return true;
}

//===============================================================================
bool VulkanContext::CreateSwapChain()
{
  const auto& physicalDevice   = GetPhysicalDevice();
  const auto& surface          = GetSurface();
  const auto& device           = GetDevice();
  const auto  indices          = VulkanContext::FindQueueFamilies(physicalDevice, surface);
  const auto  swapchainSupport = VulkanContext::QuerySwapChainSupport(physicalDevice, surface);
  const auto  extent           = GetExtent();

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
vk::SurfaceFormatKHR VulkanContext::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
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
vk::PresentModeKHR VulkanContext::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
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
bool VulkanContext::CreateRenderPass()
{
  vk::AttachmentDescription colorAttachment = {};
  colorAttachment.format                    = m_SwapChainImageFormat;
  colorAttachment.samples                   = vk::SampleCountFlagBits::e1;
  colorAttachment.loadOp                    = vk::AttachmentLoadOp::eClear;
  colorAttachment.storeOp                   = vk::AttachmentStoreOp::eStore;
  colorAttachment.stencilStoreOp            = vk::AttachmentStoreOp::eDontCare;
  colorAttachment.stencilLoadOp             = vk::AttachmentLoadOp::eDontCare;
  colorAttachment.initialLayout             = vk::ImageLayout::eUndefined;
  colorAttachment.finalLayout               = vk::ImageLayout::ePresentSrcKHR;

  vk::AttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment              = 0;
  colorAttachmentRef.layout                  = vk::ImageLayout::eColorAttachmentOptimal;

  vk::SubpassDescription subpass = {};
  subpass.pipelineBindPoint      = vk::PipelineBindPoint::eGraphics;
  subpass.colorAttachmentCount   = 1;
  subpass.pColorAttachments      = &colorAttachmentRef;

  vk::SubpassDependency dependency = {};
  dependency.srcSubpass            = vk::SubpassExternal;
  dependency.dstSubpass            = 0;
  dependency.srcStageMask          = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  dependency.srcAccessMask         = vk::AccessFlagBits::eNone;
  dependency.dstStageMask          = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  dependency.dstAccessMask         = vk::AccessFlagBits::eColorAttachmentWrite;

  vk::RenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType                    = vk::StructureType::eRenderPassCreateInfo;
  renderPassInfo.attachmentCount          = 1;
  renderPassInfo.pAttachments             = &colorAttachment;
  renderPassInfo.subpassCount             = 1;
  renderPassInfo.pSubpasses               = &subpass;
  renderPassInfo.dependencyCount          = 1;
  renderPassInfo.pDependencies            = &dependency;

  if (auto result = m_Device.createRenderPass(&renderPassInfo, nullptr, &m_RenderPass); result != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to create render pass!");
    return false;
  }
  return true;
}

//===============================================================================
bool VulkanContext::CreateGraphicsPipeline()
{
  auto verShaderCode  = ReadFile("shaders/simpleShader.vert.spv");
  auto fragShaderCode = ReadFile("shaders/simpleShader.frag.spv");

  auto vertShaderModule = CreateShaderModule(verShaderCode);
  auto fragShaderModule = CreateShaderModule(fragShaderCode);

  vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType  = vk::StructureType::ePipelineShaderStageCreateInfo;
  vertShaderStageInfo.stage  = vk::ShaderStageFlagBits::eVertex;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName  = "main";

  vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType  = vk::StructureType::ePipelineShaderStageCreateInfo;
  fragShaderStageInfo.stage  = vk::ShaderStageFlagBits::eFragment;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName  = "main";

  std::array shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

  std::vector dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

  vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
  dynamicStateInfo.sType             = vk::StructureType::ePipelineDynamicStateCreateInfo;
  dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicStateInfo.pDynamicStates    = dynamicStates.data();


  vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType                           = vk::StructureType::ePipelineVertexInputStateCreateInfo;
  vertexInputInfo.vertexBindingDescriptionCount   = 0;
  vertexInputInfo.pVertexBindingDescriptions      = nullptr; // Optional
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions    = nullptr; // Optional

  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
  inputAssemblyInfo.sType                  = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
  inputAssemblyInfo.topology               = vk::PrimitiveTopology::eTriangleList;
  inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

  vk::Viewport viewport{};
  viewport.x        = 0.0f;
  viewport.y        = 0.0f;
  viewport.width    = static_cast<float>(GetExtent().width);
  viewport.height   = static_cast<float>(GetExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vk::Rect2D scissor{};
  scissor.offset = vk::Offset2D{0, 0};
  scissor.extent = GetExtent();

  vk::PipelineViewportStateCreateInfo viewportStateInfo{};
  viewportStateInfo.sType         = vk::StructureType::ePipelineViewportStateCreateInfo;
  viewportStateInfo.viewportCount = 1;
  viewportStateInfo.pViewports    = &viewport;
  viewportStateInfo.scissorCount  = 1;
  viewportStateInfo.pScissors     = &scissor;

  vk::PipelineRasterizationStateCreateInfo rasterizerInfo{};
  rasterizerInfo.sType                   = vk::StructureType::ePipelineRasterizationStateCreateInfo;
  rasterizerInfo.depthClampEnable        = VK_FALSE;
  rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
  rasterizerInfo.polygonMode             = vk::PolygonMode::eFill;
  rasterizerInfo.lineWidth               = 1.0f;
  rasterizerInfo.cullMode                = vk::CullModeFlagBits::eBack;
  rasterizerInfo.frontFace               = vk::FrontFace::eCounterClockwise;
  rasterizerInfo.depthBiasEnable         = VK_FALSE;

  vk::PipelineMultisampleStateCreateInfo multisamplingInfo{};
  multisamplingInfo.sType                = vk::StructureType::ePipelineMultisampleStateCreateInfo;
  multisamplingInfo.sampleShadingEnable  = VK_FALSE;
  multisamplingInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;

  vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  colorBlendAttachment.blendEnable         = VK_TRUE;
  colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
  colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
  colorBlendAttachment.colorBlendOp        = vk::BlendOp::eAdd;
  colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
  colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
  colorBlendAttachment.alphaBlendOp        = vk::BlendOp::eAdd;

  vk::PipelineColorBlendStateCreateInfo colorBlendingInfo{};
  colorBlendingInfo.sType             = vk::StructureType::ePipelineColorBlendStateCreateInfo;
  colorBlendingInfo.logicOpEnable     = VK_FALSE;
  colorBlendingInfo.logicOp           = vk::LogicOp::eCopy;
  colorBlendingInfo.attachmentCount   = 1;
  colorBlendingInfo.pAttachments      = &colorBlendAttachment;
  colorBlendingInfo.blendConstants[0] = 0.0f; // Optional
  colorBlendingInfo.blendConstants[1] = 0.0f; // Optional
  colorBlendingInfo.blendConstants[2] = 0.0f; // Optional
  colorBlendingInfo.blendConstants[3] = 0.0f; // Optional

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType                  = vk::StructureType::ePipelineLayoutCreateInfo;
  pipelineLayoutInfo.setLayoutCount         = 0;       // Optional
  pipelineLayoutInfo.pSetLayouts            = nullptr; // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0;       // Optional
  pipelineLayoutInfo.pPushConstantRanges    = nullptr; // Optional

  if (m_Device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_PipelineLayout) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("Failed to create pipeline layout");
    return false;
  }

  vk::GraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.stageCount          = 2;
  pipelineInfo.pStages             = shaderStages.data();
  pipelineInfo.pVertexInputState   = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
  pipelineInfo.pViewportState      = &viewportStateInfo;
  pipelineInfo.pRasterizationState = &rasterizerInfo;
  pipelineInfo.pMultisampleState   = &multisamplingInfo;
  pipelineInfo.pColorBlendState    = &colorBlendingInfo;
  pipelineInfo.pDynamicState       = &dynamicStateInfo;
  pipelineInfo.layout              = m_PipelineLayout;
  pipelineInfo.renderPass          = m_RenderPass;
  pipelineInfo.subpass             = 0;
  pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;

  if (m_Device.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("Failed to create graphics pipeline");
    return false;
  }

  m_Device.destroyShaderModule(fragShaderModule);
  m_Device.destroyShaderModule(vertShaderModule);

  return true;
}

//===============================================================================
vk::ShaderModule VulkanContext::CreateShaderModule(const std::vector<char>& code)
{
  vk::ShaderModuleCreateInfo createInfo{};
  createInfo.sType    = vk::StructureType::eShaderModuleCreateInfo;
  createInfo.codeSize = code.size();
  createInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());
  return m_Device.createShaderModule(createInfo);
}

//===============================================================================
vk::Format VulkanContext::FindDepthFormat()
{
  return vk::Format::eD32Sfloat;
  // return m_VulkanContext.FindSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
  //                                            vk::ImageTiling::eOptimal,
  //                                            vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

//===============================================================================
bool VulkanContext::CreateFramebuffers()
{
  m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());
  for (size_t i = 0; i < m_SwapChainFramebuffers.size(); ++i)
  {
    std::array attachments = {m_SwapChainImageViews[i]};

    vk::FramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType           = vk::StructureType::eFramebufferCreateInfo;
    framebufferInfo.renderPass      = m_RenderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments    = attachments.data();
    framebufferInfo.width           = GetExtent().width;
    framebufferInfo.height          = GetExtent().height;
    framebufferInfo.layers          = 1;

    if (m_Device.createFramebuffer(&framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != vk::Result::eSuccess)
    {
      LOG_CORE_ERROR("failed to create framebuffer!");
      return false;
    }
  }
  return true;
}

//===============================================================================
bool VulkanContext::CreateCommandPool()
{
  QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_PhysicalDevice, m_Surface);

  vk::CommandPoolCreateInfo poolInfo{};
  poolInfo.sType            = vk::StructureType::eCommandPoolCreateInfo;
  poolInfo.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  if (m_Device.createCommandPool(&poolInfo, nullptr, &m_CommandPool) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("Failed to create command pool");
    return false;
  }
  return true;
}

//===============================================================================
bool VulkanContext::CreateCommandBuffers()
{
  vk::CommandBufferAllocateInfo allocInfo{};
  allocInfo.sType              = vk::StructureType::eCommandBufferAllocateInfo;
  allocInfo.commandPool        = m_CommandPool;
  allocInfo.level              = vk::CommandBufferLevel::ePrimary;
  allocInfo.commandBufferCount = 1;

  if (m_Device.allocateCommandBuffers(&allocInfo, &m_CommandBuffer) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("Failed to allocate command buffers");
    return false;
  }
  return true;
}

//===============================================================================
bool VulkanContext::CreateSyncObjects()
{
  vk::SemaphoreCreateInfo semaphoreInfo{};
  vk::FenceCreateInfo     fenceInfo{vk::FenceCreateFlagBits::eSignaled};
  if (m_Device.createSemaphore(&semaphoreInfo, nullptr, &m_ImageAvailableSemaphore) != vk::Result::eSuccess ||
      m_Device.createSemaphore(&semaphoreInfo, nullptr, &m_RenderFinishedSemaphore) != vk::Result::eSuccess ||
      m_Device.createFence(&fenceInfo, nullptr, &m_InFlightFence) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("Failed to create synchronization objects for a frame");
    return false;
  }
  return true;
}

//===============================================================================
void VulkanContext::RecordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const
{
  vk::CommandBufferBeginInfo beginInfo{};
  commandBuffer.begin(beginInfo);

  vk::ClearValue          clearColor{{0.0f, 0.0f, 0.0f, 1.0f}};
  vk::RenderPassBeginInfo renderPassInfo{m_RenderPass,
                                         m_SwapChainFramebuffers[imageIndex],
                                         vk::Rect2D{vk::Offset2D{0, 0}, GetExtent()},
                                         1,
                                         &clearColor};

  //begin render pass
  commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

  // basic draw
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline);

  vk::Viewport viewport{0.0f, 0.0f, static_cast<float>(GetExtent().width), static_cast<float>(GetExtent().height), 0.0f, 1.0f};
  commandBuffer.setViewport(0, viewport);

  vk::Rect2D scissor{vk::Offset2D{0, 0}, GetExtent()};
  commandBuffer.setScissor(0, scissor);

  commandBuffer.draw(3, 1, 0, 0);

  //end render pass
  commandBuffer.endRenderPass();

  commandBuffer.end();
}

//===============================================================================
void VulkanContext::oldDrawFrame()
{
  // vk::Semaphore imageAcquiredSemaphore = m_Device.createSemaphore(vk::SemaphoreCreateInfo());
  // vk::Fence     drawFence              = m_Device.createFence(vk::FenceCreateInfo());
  //
  // vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
  // vk::SubmitInfo         submitInfo(imageAcquiredSemaphore, waitDestinationStageMask, m_CommandBuffer);
  // m_GraphicsQueue.submit(submitInfo, drawFence);
  //
  // while (vk::Result::eTimeout == m_Device.waitForFences(drawFence, VK_TRUE, vk::su::FenceTimeout))
  //   ;
  //
  //
  // vk::Result result = m_PresentQueue.presentKHR(vk::PresentInfoKHR({}, swapChainData.swapChain, currentBuffer.value));
  // switch (result)
  // {
  //   case vk::Result::eSuccess:
  //     break;
  //   case vk::Result::eSuboptimalKHR:
  //     std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
  //     break;
  //   default:
  //     assert(false); // an unexpected result is returned !
  // }
}

void VulkanContext::DrawFrame()
{
  while (vk::Result::eTimeout == m_Device.waitForFences(1, &m_InFlightFence, VK_TRUE, std::numeric_limits<uint64_t>::max()))
    ;


  if (m_Device.resetFences(1, &m_InFlightFence) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to reset fence!");
    return;
  }

  uint32_t imageIndex{};
  if (const vk::Result result = m_Device.acquireNextImageKHR(m_SwapChain,
                                                             std::numeric_limits<uint64_t>::max(),
                                                             m_ImageAvailableSemaphore,
                                                             nullptr,
                                                             &imageIndex);
      (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR))
  {
    LOG_CORE_ERROR("failed to acquire next swap chain image!");
  }
  m_CommandBuffer.reset();
  RecordCommandBuffer(m_CommandBuffer, imageIndex);

  vk::SubmitInfo submitInfo{};

  std::array                            waitSemaphores = {m_ImageAvailableSemaphore};
  std::array<vk::PipelineStageFlags, 1> waitStages     = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
  submitInfo.waitSemaphoreCount                        = 1;
  submitInfo.pWaitSemaphores                           = waitSemaphores.data();
  submitInfo.pWaitDstStageMask                         = waitStages.data();

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &m_CommandBuffer;

  std::array signalSemaphores     = {m_RenderFinishedSemaphore};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores    = signalSemaphores.data();

  m_GraphicsQueue.submit(submitInfo, m_InFlightFence);

  vk::PresentInfoKHR presentInfo{};
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores    = signalSemaphores.data();

  std::array swapChains      = {m_SwapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains    = swapChains.data();
  presentInfo.pImageIndices  = &imageIndex;
  presentInfo.pResults       = nullptr;
  if (m_PresentQueue.presentKHR(presentInfo) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to present swap chain image!");
  }
}
} // namespace four
