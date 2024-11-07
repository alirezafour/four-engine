#include "four-pch.hpp"

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
  return CreateInstance() && SetupDebugMessenger() && CreateSurface() && PickPhysicalDevice() &&
         CreateLogicalDevice() && CreateSwapChain() && CreateImageViews() && CreateRenderPass() &&
         CreateGraphicsPipeline() && CreateFramebuffers() && CreateCommandPool() && CreateVertexBuffers() &&
         CreateCommandBuffers() && CreateSyncObjects();
}

//===============================================================================
void VulkanContext::Shutdown()
{
  if (m_Device)
  {
    CleanupSwapChain();
    m_Device.destroyBuffer(m_VertexBuffer);
    m_Device.freeMemory(m_VertexBufferMemory);

    m_Device.destroyPipelineLayout(m_PipelineLayout);
    m_Device.destroyPipeline(m_GraphicsPipeline);

    m_Device.destroyRenderPass(m_RenderPass);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
      m_Device.destroySemaphore(m_ImageAvailableSemaphores[i]);
      m_Device.destroySemaphore(m_RenderFinishedSemaphores[i]);
      m_Device.destroyFence(m_InFlightFences[i]);
    }

    m_Device.destroyCommandPool(m_CommandPool);

    m_Device.destroy();
  }
  if (m_Instance)
  {
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
  m_PhysicalDevice           = VK_NULL_HANDLE;
  const auto physicalDevices = m_Instance.enumeratePhysicalDevices();
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
  const std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
  for (float queuePriority = 1.f; const auto& queueFamily : uniqueQueueFamilies)
  {
    queueCreateInfos.push_back({{}, queueFamily, 1, &queuePriority});
  }
  const auto features = m_PhysicalDevice.getFeatures();

  m_Device = m_PhysicalDevice.createDevice(
    {{},
     static_cast<uint32_t>(queueCreateInfos.size()),
     queueCreateInfos.data(),
     {},
     {},
     static_cast<uint32_t>(m_DeviceExtensions.size()),
     m_DeviceExtensions.data(),
     &features});
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
  const std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
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
  const std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

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
  const auto queueFamilies = device.getQueueFamilyProperties();

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

  const auto properties = device.getProperties();
  const auto features   = device.getFeatures();
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
    m_SwapChainImageViews[i] = GetDevice().createImageView(
      {{},
       m_SwapChainImages[i],
       vk::ImageViewType::e2D,
       m_SwapChainImageFormat,
       {vk::ComponentSwizzle::eIdentity,  // r
        vk::ComponentSwizzle::eIdentity,  // g
        vk::ComponentSwizzle::eIdentity,  // b
        vk::ComponentSwizzle::eIdentity}, // a
       {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}});
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
  SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice, m_Surface);

  const auto surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
  const auto presentMode   = ChooseSwapPresentMode(swapChainSupport.presentModes);
  const auto extent        = ChooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
  {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR
    createInfo{{},
               m_Surface,
               imageCount,
               surfaceFormat.format,
               surfaceFormat.colorSpace,
               extent,
               1,
               vk::ImageUsageFlagBits::eColorAttachment,
               vk::SharingMode::eExclusive,
               {},
               {},
               swapChainSupport.capabilities.currentTransform,
               vk::CompositeAlphaFlagBitsKHR::eOpaque,
               presentMode,
               VK_TRUE,
               VK_NULL_HANDLE};

  QueueFamilyIndices      indices            = FindQueueFamilies(m_PhysicalDevice, m_Surface);
  std::array<uint32_t, 2> queueFamilyIndices = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily)
  {
    createInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
  }

  m_SwapChain = m_Device.createSwapchainKHR(createInfo);
  if (!m_SwapChain)
  {
    LOG_CORE_ERROR("failed to create swap chain!");
    return false;
  }

  m_SwapChainImages = m_Device.getSwapchainImagesKHR(m_SwapChain);
  if (m_SwapChainImages.empty())
  {
    LOG_CORE_ERROR("failed to get swap chain images!");
    return false;
  }

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
  const vk::AttachmentDescription
                                 colorAttachment{{},
                    m_SwapChainImageFormat,
                    vk::SampleCountFlagBits::e1,
                    vk::AttachmentLoadOp::eClear,
                    vk::AttachmentStoreOp::eStore,
                    vk::AttachmentLoadOp::eDontCare,
                    vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined,
                    vk::ImageLayout::ePresentSrcKHR};
  const vk::AttachmentReference  colorAttachmentRef{0, vk::ImageLayout::eColorAttachmentOptimal};
  const vk::SubpassDescription   subpass{{}, vk::PipelineBindPoint::eGraphics, {}, {}, 1, &colorAttachmentRef};
  const vk::SubpassDependency    dependency{vk::SubpassExternal,
                                         0,
                                         vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                         vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                         vk::AccessFlagBits::eNone,
                                         vk::AccessFlagBits::eColorAttachmentWrite};
  const vk::RenderPassCreateInfo renderPassInfo{{}, 1, &colorAttachment, 1, &subpass, 1, &dependency};
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
  const auto verShaderCode  = ReadFile("shaders/simpleShader.vert.spv");
  const auto fragShaderCode = ReadFile("shaders/simpleShader.frag.spv");

  const auto vertShaderModule = CreateShaderModule(verShaderCode);
  const auto fragShaderModule = CreateShaderModule(fragShaderCode);

  const vk::PipelineShaderStageCreateInfo
    vertShaderStageInfo{{}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main"};
  const vk::PipelineShaderStageCreateInfo
                   fragShaderStageInfo{{}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main"};
  const std::array shaderStages          = {vertShaderStageInfo, fragShaderStageInfo};
  const auto       bindingDescription    = Vertex::GetBindingDescription();
  const auto       attributeDescriptions = Vertex::GetAttributeDescriptions();
  const vk::PipelineVertexInputStateCreateInfo   vertexInputInfo{{},
                                                               1,
                                                               &bindingDescription,
                                                               static_cast<uint32_t>(attributeDescriptions.size()),
                                                               attributeDescriptions.data()};
  const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{{}, vk::PrimitiveTopology::eTriangleList, VK_FALSE};

  const vk::PipelineViewportStateCreateInfo viewportStateInfo{{}, 1, nullptr, 1, nullptr};

  const vk::PipelineRasterizationStateCreateInfo
    rasterizerInfo{{},
                   VK_FALSE,
                   VK_FALSE,
                   vk::PolygonMode::eFill,
                   vk::CullModeFlagBits::eBack,
                   vk::FrontFace::eClockwise,
                   VK_FALSE,
                   {},
                   {},
                   1.0f};

  const vk::PipelineMultisampleStateCreateInfo multisamplingInfo{{}, vk::SampleCountFlagBits::e1, VK_FALSE};

  vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  colorBlendAttachment.blendEnable = VK_FALSE;

  const vk::PipelineColorBlendStateCreateInfo
    colorBlendingInfo{{}, VK_FALSE, vk::LogicOp::eCopy, 1, &colorBlendAttachment, {0.0f, 0.0f, 0.0f, 0.0f}};
  const std::vector                        dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
  const vk::PipelineDynamicStateCreateInfo dynamicStateInfo{{},
                                                            static_cast<uint32_t>(dynamicStates.size()),
                                                            dynamicStates.data()};
  const vk::PipelineLayoutCreateInfo       pipelineLayoutInfo{{}, 0, {}, 0};
  if (m_Device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_PipelineLayout) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("Failed to create pipeline layout");
    return false;
  }

  const vk::GraphicsPipelineCreateInfo
    pipelineInfo{{},
                 2,
                 shaderStages.data(),
                 &vertexInputInfo,
                 &inputAssemblyInfo,
                 nullptr,
                 &viewportStateInfo,
                 &rasterizerInfo,
                 &multisamplingInfo,
                 nullptr,
                 &colorBlendingInfo,
                 &dynamicStateInfo,
                 m_PipelineLayout,
                 m_RenderPass,
                 0,
                 VK_NULL_HANDLE};

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
  return m_Device.createShaderModule({{}, code.size(), reinterpret_cast<const uint32_t*>(code.data())});
}

//===============================================================================
vk::Format VulkanContext::FindDepthFormat() const
{
  return FindSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                             vk::ImageTiling::eOptimal,
                             vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

//===============================================================================
bool VulkanContext::CreateFramebuffers()
{
  const auto& extent = GetExtent();
  m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());
  for (size_t i = 0; i < m_SwapChainImageViews.size(); ++i)
  {
    const std::array attachments = {m_SwapChainImageViews[i]};
    const vk::FramebufferCreateInfo framebufferInfo{{}, m_RenderPass, 1, attachments.data(), extent.width, extent.height, 1};

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
  const QueueFamilyIndices queueFamilyIndices{FindQueueFamilies(m_PhysicalDevice, m_Surface)};

  const vk::CommandPoolCreateInfo poolInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                           queueFamilyIndices.graphicsFamily.value()};

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
  m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  const vk::CommandBufferAllocateInfo allocInfo{m_CommandPool,
                                                vk::CommandBufferLevel::ePrimary,
                                                static_cast<uint32_t>(m_CommandBuffers.size())};

  if (m_Device.allocateCommandBuffers(&allocInfo, m_CommandBuffers.data()) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("Failed to allocate command buffers");
    return false;
  }
  return true;
}

//===============================================================================
bool VulkanContext::CreateSyncObjects()
{
  m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  vk::SemaphoreCreateInfo semaphoreInfo{};
  vk::FenceCreateInfo     fenceInfo{vk::FenceCreateFlagBits::eSignaled};
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
  {
    if (m_Device.createSemaphore(&semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != vk::Result::eSuccess ||
        m_Device.createSemaphore(&semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != vk::Result::eSuccess ||
        m_Device.createFence(&fenceInfo, nullptr, &m_InFlightFences[i]) != vk::Result::eSuccess)
    {
      LOG_CORE_ERROR("Failed to create synchronization objects for a frame");
      return false;
    }
  }
  return true;
}

//===============================================================================
bool VulkanContext::CreateVertexBuffers()
{
  vk::BufferCreateInfo bufferInfo{{},
                                  sizeof(vertices) * vertices.size(),
                                  vk::BufferUsageFlagBits::eVertexBuffer,
                                  vk::SharingMode::eExclusive};

  if (m_Device.createBuffer(&bufferInfo, nullptr, &m_VertexBuffer) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to create vertex buffer!");
    return false;
  }

  const vk::MemoryRequirements memRequirment = m_Device.getBufferMemoryRequirements(m_VertexBuffer);
  const vk::MemoryAllocateInfo allocInfo{memRequirment.size,
                                         FindMemoryType(memRequirment.memoryTypeBits,
                                                        vk::MemoryPropertyFlagBits::eHostVisible |
                                                          vk::MemoryPropertyFlagBits::eHostCoherent)};
  if (m_Device.allocateMemory(&allocInfo, nullptr, &m_VertexBufferMemory) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to allocate vertex buffer memory!");
    return false;
  }
  m_Device.bindBufferMemory(m_VertexBuffer, m_VertexBufferMemory, 0);

  void* data;
  if (m_Device.mapMemory(m_VertexBufferMemory, 0, bufferInfo.size, {}, &data) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to map vertex buffer memory!");
    return false;
  }
  memcpy(data, vertices.data(), static_cast<size_t>(bufferInfo.size));
  m_Device.unmapMemory(m_VertexBufferMemory);

  return true;
}

//===============================================================================
void VulkanContext::ReCreateSwapChain()
{
  while (m_Window.GetWidth() == 0 || m_Window.GetHeight() == 0)
  {
    m_Window.WaitEvents();
  }
  m_Device.waitIdle();
  CleanupSwapChain();
  if (CreateSwapChain() && CreateImageViews() && CreateFramebuffers())
  {
    // success
    return;
  }

  LOG_CORE_ERROR("failed to recreate swap chain!");
}

//===============================================================================
void VulkanContext::CleanupSwapChain()
{
  for (auto& framebuffer : m_SwapChainFramebuffers)
  {
    m_Device.destroyFramebuffer(framebuffer);
  }

  for (auto& imageView : m_SwapChainImageViews)
  {
    m_Device.destroyImageView(imageView);
  }
  m_Device.destroySwapchainKHR(m_SwapChain);
}

//===============================================================================
void VulkanContext::RecordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const
{
  const auto&                      extent = GetExtent();
  const vk::CommandBufferBeginInfo beginInfo{};
  commandBuffer.begin(beginInfo);

  const vk::ClearValue clearColor{{0.02f, 0.02f, 0.02f, 1.0f}};

  //begin render pass
  commandBuffer
    .beginRenderPass({m_RenderPass, m_SwapChainFramebuffers[imageIndex], vk::Rect2D{vk::Offset2D{0, 0}, extent}, 1, &clearColor},
                     vk::SubpassContents::eInline);

  // basic draw
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline);
  vk::Buffer     vertexBuffers[] = {m_VertexBuffer};
  vk::DeviceSize offsets[]       = {0};
  commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

  vk::Viewport viewport{0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f};
  commandBuffer.setViewport(0, 1, &viewport);

  vk::Rect2D scissor{vk::Offset2D{0, 0}, extent};
  commandBuffer.setScissor(0, 1, &scissor);

  commandBuffer.draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0);

  //end render pass
  commandBuffer.endRenderPass();

  commandBuffer.end();
}

//===============================================================================
void VulkanContext::DrawFrame()
{
  while (vk::Result::eTimeout ==
         m_Device.waitForFences(1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max()))
  {
  }

  uint32_t imageIndex{};
  if (const vk::Result result = m_Device.acquireNextImageKHR(m_SwapChain,
                                                             std::numeric_limits<uint64_t>::max(),
                                                             m_ImageAvailableSemaphores[m_CurrentFrame],
                                                             nullptr,
                                                             &imageIndex);
      result == vk::Result::eErrorOutOfDateKHR)
  {
    ReCreateSwapChain();
    return;
  }
  else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
  {
    LOG_CORE_ERROR("failed to acquire next swap chain image!");
  }

  if (m_Device.resetFences(1, &m_InFlightFences[m_CurrentFrame]) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to reset fence!");
    return;
  }

  m_CommandBuffers[m_CurrentFrame].reset();
  RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);

  std::array                            waitSemaphores   = {m_ImageAvailableSemaphores[m_CurrentFrame]};
  std::array<vk::PipelineStageFlags, 1> waitStages       = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
  std::array                            signalSemaphores = {m_RenderFinishedSemaphores[m_CurrentFrame]};

  vk::SubmitInfo
    submitInfo{1, waitSemaphores.data(), waitStages.data(), 1, &m_CommandBuffers[m_CurrentFrame], 1, signalSemaphores.data()};
  m_GraphicsQueue.submit(submitInfo, m_InFlightFences[m_CurrentFrame]);

  std::array swapChains = {m_SwapChain};
  if (const vk::Result result = m_PresentQueue.presentKHR(
        {1, signalSemaphores.data(), 1, swapChains.data(), &imageIndex, nullptr});
      result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_Window.WasWindowResized())
  {
    ReCreateSwapChain();
  }
  else if (result != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to present swap chain image!");
  }

  m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

//===============================================================================
void VulkanContext::StopRender()
{
  m_Device.waitIdle();
}

//===============================================================================
uint32_t VulkanContext::FindMemoryType(uint32_t typeFilter, const vk::MemoryPropertyFlags& properties) const
{
  vk::PhysicalDeviceMemoryProperties memProperties = m_PhysicalDevice.getMemoryProperties();
  for (size_t i = 0; i < memProperties.memoryTypeCount; ++i)
  {
    if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
    {
      return i;
    }
  }

  LOG_CORE_ERROR("failed to find suitable memory type!");
  return {};
}
} // namespace four
