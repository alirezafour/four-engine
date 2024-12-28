// module vkContext;
#include "four-pch.hpp"

#include "renderer/vulkan/vulkanRenderer.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "window/glfw/glfwWindow.hpp"
#include "GLFW/glfw3.h"

#include "glm/gtc/matrix_transform.hpp"

#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace four
{

//===============================================================================
static std::vector<char> ReadFile(const std::string& filename)
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
VulkanRenderer::VulkanRenderer(WindowType& window) :
m_Window{window},
m_MainCamera{{1.0F, 2.0F, 3.5F}, -135.5F, -34.0F, {0.0F, 0.0F, 0.0F}}
{
  LOG_CORE_INFO("Initializing Vulkan context.");
  const bool result = InitVulkan();
  m_MainCamera.SetupEvents(m_Window);

  if (!result)
  {
    LOG_CORE_ERROR("Failed to initialize Vulkan context.");
  }
}

//===============================================================================
VulkanRenderer::~VulkanRenderer()
{
  ShutdownVulkan();
}

//===============================================================================
bool VulkanRenderer::InitVulkan()
{
  try
  {
    return CreateInstance() &&            //
           SetupDebugMessenger() &&       //
           CreateSurface() &&             //
           PickPhysicalDevice() &&        //
           CreateLogicalDevice() &&       //
           CreateSwapChain() &&           //
           CreateImageViews() &&          //
           CreateRenderPass() &&          //
           CreateDescriptorSetLayout() && //
           CreateGraphicsPipeline() &&    //
           CreateCommandPool() &&         //
           CreateDepthResources() &&      //
           CreateFramebuffers() &&        //
           CreateTextureImage() &&        //
           CreateTextureImageView() &&    //
           CreateTextureSampler() &&      //
           CreateVertexBuffers() &&       //
           CreateIndexBuffers() &&        //
           CreateUniformBuffers() &&      //
           CreateDescriptorPool() &&      //
           CreateDescriptorSets() &&      //
           CreateCommandBuffers() &&      //
           CreateSyncObjects() &&         //
           InitImGui();
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR("exception caught: {}", e.what());
  }
  return false;
}

//===============================================================================
void VulkanRenderer::ShutdownVulkan()
{
  if (m_Device)
  {
    CleanupSwapChain();

    m_Device.destroySampler(m_TextureSampler);
    m_Device.destroyImageView(m_TextureImageView);

    m_Device.destroyImage(m_TextureImage);
    m_Device.freeMemory(m_TextureImageMemory);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
      m_Device.destroyBuffer(m_UniformBuffers[i]);
      m_Device.freeMemory(m_UniformBuffersMemory[i]);
    }

    m_Device.destroyDescriptorPool(m_DescriptorPool);
    m_Device.destroyDescriptorSetLayout(m_DescriptorSetLayout);
    m_Device.destroyBuffer(m_VertexBuffer);
    m_Device.freeMemory(m_VertexBufferMemory);

    m_Device.destroyBuffer(m_IndexBuffer);
    m_Device.freeMemory(m_IndexBufferMemory);

    m_Device.destroyPipelineLayout(m_PipelineLayout);
    m_Device.destroyPipeline(m_GraphicsPipeline);

    m_Device.destroyRenderPass(m_RenderPass);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
      m_Device.destroyFence(m_FrameData[i].inFlightFence);
      m_Device.destroySemaphore(m_FrameData[i].imageAvailableSemaphore);
      m_Device.destroySemaphore(m_FrameData[i].renderFinishedSemaphore);
    }
    for (auto& frameData : m_FrameData)
    {
      frameData.deletionQueue.flush();
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
void VulkanRenderer::ImmediateSubmit(std::function<void(vk::CommandBuffer commandBuffer)>&& function)
{
  m_Device.resetFences(m_ImmediateFence);
  m_ImmediateCommandBuffer.reset();
  vk::CommandBuffer cmd = m_ImmediateCommandBuffer;
  cmd.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  function(cmd);
  cmd.end();
  vk::CommandBufferSubmitInfo cmdSubmitInfo{.commandBuffer = cmd};
  vk::SubmitInfo2             submitInfo{.commandBufferInfoCount = 1, .pCommandBufferInfos = &cmdSubmitInfo};
  const auto                  result = m_GraphicsQueue.submit2(1, &submitInfo, m_ImmediateFence);
  [[maybe_unused]] const auto result2 = m_Device.waitForFences(m_ImmediateFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
}

//===============================================================================
bool VulkanRenderer::CreateInstance()
{
  if (EnableValidationLayers && !CheckValidationLayerSupport())
  {
    LOG_CORE_ERROR("validation layers requested, but not available!");
    return false;
  }

  vk::ApplicationInfo    appInfo{.pApplicationName   = "four-engine",
                                 .applicationVersion = 1,
                                 .pEngineName        = "four-engine",
                                 .engineVersion      = 1,
                                 .apiVersion         = VK_API_VERSION_1_3};
  vk::InstanceCreateInfo createInfo{.flags = vk::InstanceCreateFlags(), .pApplicationInfo = &appInfo};

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
bool VulkanRenderer::SetupDebugMessenger()
{
  if (!EnableValidationLayers)
  {
    return true;
  }
  return true;
}

//===============================================================================
bool VulkanRenderer::CreateSurface()
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
bool VulkanRenderer::PickPhysicalDevice()
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
bool VulkanRenderer::CreateLogicalDevice()
{
  const auto indices = FindQueueFamilies(m_PhysicalDevice, m_Surface);

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  const std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
  queueCreateInfos.reserve(uniqueQueueFamilies.size());
  for (float queuePriority = 1.F; const auto& queueFamily : uniqueQueueFamilies)
  {
    queueCreateInfos.push_back(
      {.flags = {}, .queueFamilyIndex = queueFamily, .queueCount = 1, .pQueuePriorities = &queuePriority});
  }
  auto features              = m_PhysicalDevice.getFeatures();
  features.samplerAnisotropy = VK_TRUE;

  m_Device = m_PhysicalDevice.createDevice(
    {.flags                   = {},
     .queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size()),
     .pQueueCreateInfos       = queueCreateInfos.data(),
     .enabledExtensionCount   = static_cast<uint32_t>(m_DeviceExtensions.size()),
     .ppEnabledExtensionNames = m_DeviceExtensions.data(),
     .pEnabledFeatures        = &features});
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
bool VulkanRenderer::CheckValidationLayerSupport()
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
std::vector<const char*> VulkanRenderer::GetRequiredExtensions()
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
bool VulkanRenderer::IsDeviceSuitable(const vk::PhysicalDevice& device) const
{
  const auto indices             = FindQueueFamilies(device, m_Surface);
  const bool extensionsSupported = CheckDeviceExtensionSupport(device);
  bool       swapChainAquate     = false;
  if (extensionsSupported)
  {
    const auto swapChainSupport = QuerySwapChainSupport(device, m_Surface);
    swapChainAquate             = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }
  const auto supportedFeatures = device.getFeatures();

  return indices.IsComplete() && extensionsSupported && swapChainAquate && (supportedFeatures.samplerAnisotropy != 0U);
}

//===============================================================================
bool VulkanRenderer::CheckDeviceExtensionSupport(const vk::PhysicalDevice& device) const
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
VulkanRenderer::SwapChainSupportDetails VulkanRenderer::QuerySwapChainSupport(const vk::PhysicalDevice& device,
                                                                              const vk::SurfaceKHR&     surface)
{
  return {.capabilities = device.getSurfaceCapabilitiesKHR(surface),
          .formats      = device.getSurfaceFormatsKHR(surface),
          .presentModes = device.getSurfacePresentModesKHR(surface)};
}

//===============================================================================
VulkanRenderer::QueueFamilyIndices VulkanRenderer::FindQueueFamilies(const vk::PhysicalDevice& device,
                                                                     const vk::SurfaceKHR&     surface)
{
  // logic to find queue families
  const auto queueFamilies = device.getQueueFamilyProperties();

  QueueFamilyIndices indices;
  for (uint32_t index = 0U; const auto& queueFamily : queueFamilies)
  {
    if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
    {
      indices.graphicsFamily = index;
      if (device.getSurfaceSupportKHR(index, surface) != 0U)
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
vk::Format VulkanRenderer::FindSupportedFormat(const std::vector<vk::Format>& candidates,
                                               vk::ImageTiling                tiling,
                                               vk::FormatFeatureFlags         features) const
{
  for (const auto& format : candidates)
  {
    const vk::FormatProperties props = m_PhysicalDevice.getFormatProperties(format);
    if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
    {
      return format;
    }
    if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
    {
      return format;
    }
  }

  LOG_CORE_ERROR("failed to find supported format!");
  return {};
}
//===============================================================================
uint32_t VulkanRenderer::RateDeviceSuitability(const vk::PhysicalDevice& device) const
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
  if (features.geometryShader == 0U)
  {
    return 0;
  }
  return score;
}

//===============================================================================
void VulkanRenderer::PrintExtensionsSupport()
{
  LOG_CORE_INFO("available extensions:");
  const std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();
  for (const auto& extension : availableExtensions)
  {
    LOG_CORE_INFO("\tExtension: {}", extension.extensionName);
  }
}

//===============================================================================
vk::Extent2D VulkanRenderer::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
{
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
  {
    return capabilities.currentExtent;
  }
  const uint32_t width  = m_Window.GetExtent().GetWidth();
  const uint32_t height = m_Window.GetExtent().GetHeight();
  vk::Extent2D   actualExtent{.width = width, .height = height};

  actualExtent.width  = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
  actualExtent.height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

  return actualExtent;
}

//===============================================================================
bool VulkanRenderer::CreateImageViews()
{
  try
  {
    m_SwapChainImageViews.resize(m_SwapChainImages.size());

    for (size_t i = 0; i < m_SwapChainImages.size(); ++i)
    {
      m_SwapChainImageViews[i] = CreateImageView(m_SwapChainImages[i], m_SwapChainImageFormat, vk::ImageAspectFlagBits::eColor);
    }
    return true;
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR("failed to create image views. exception: {}", e.what());
  }
  return false;
}

//===============================================================================
bool VulkanRenderer::CreateSwapChain()
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
    createInfo{.flags            = {},
               .surface          = m_Surface,
               .minImageCount    = imageCount,
               .imageFormat      = surfaceFormat.format,
               .imageColorSpace  = surfaceFormat.colorSpace,
               .imageExtent      = extent,
               .imageArrayLayers = 1,
               .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
               .imageSharingMode = vk::SharingMode::eExclusive,
               .preTransform     = swapChainSupport.capabilities.currentTransform,
               .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
               .presentMode      = presentMode,
               .clipped          = VK_TRUE,
               .oldSwapchain     = VK_NULL_HANDLE};

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
bool VulkanRenderer::CreateRenderPass()
{
  const vk::AttachmentDescription colorAttachment{
    .format         = m_SwapChainImageFormat,
    .samples        = vk::SampleCountFlagBits::e1,
    .loadOp         = vk::AttachmentLoadOp::eClear,
    .storeOp        = vk::AttachmentStoreOp::eStore,
    .stencilLoadOp  = vk::AttachmentLoadOp::eDontCare,
    .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
    .initialLayout  = vk::ImageLayout::eUndefined,
    .finalLayout    = vk::ImageLayout::ePresentSrcKHR,
  };
  //depth
  const vk::AttachmentDescription depthAttachment{
    .format         = FindDepthFormat(),
    .samples        = vk::SampleCountFlagBits::e1,
    .loadOp         = vk::AttachmentLoadOp::eClear,
    .storeOp        = vk::AttachmentStoreOp::eDontCare,
    .stencilLoadOp  = vk::AttachmentLoadOp::eDontCare,
    .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
    .initialLayout  = vk::ImageLayout::eUndefined,
    .finalLayout    = vk::ImageLayout::eDepthStencilAttachmentOptimal,
  };
  const vk::AttachmentReference colorAttachmentRef{
    .attachment = 0,
    .layout     = vk::ImageLayout::eColorAttachmentOptimal,
  };
  const vk::AttachmentReference depthAttachmentRef{
    .attachment = 1,
    .layout     = vk::ImageLayout::eDepthStencilAttachmentOptimal,
  };
  const vk::SubpassDescription subpass{
    .pipelineBindPoint       = vk::PipelineBindPoint::eGraphics,
    .colorAttachmentCount    = 1,
    .pColorAttachments       = &colorAttachmentRef,
    .pDepthStencilAttachment = &depthAttachmentRef,
  };
  const vk::SubpassDependency dependency{
    .srcSubpass    = vk::SubpassExternal,
    .dstSubpass    = 0,
    .srcStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
    .dstStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
    .srcAccessMask = {},
    .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
  };


  const std::array attachments = {colorAttachment, depthAttachment};

  const vk::RenderPassCreateInfo renderPassInfo{
    .attachmentCount = static_cast<uint32_t>(attachments.size()),
    .pAttachments    = attachments.data(),
    .subpassCount    = 1,
    .pSubpasses      = &subpass,
    .dependencyCount = 1,
    .pDependencies   = &dependency,
  };
  if (auto result = m_Device.createRenderPass(&renderPassInfo, nullptr, &m_RenderPass); result != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to create render pass!");
    return false;
  }
  return true;
}

//===============================================================================
bool VulkanRenderer::CreateDescriptorSetLayout()
{
  const vk::DescriptorSetLayoutBinding                uboLayoutBinding{.binding = 0,
                                                                       .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                                       .descriptorCount = 1,
                                                                       .stageFlags = vk::ShaderStageFlagBits::eVertex,
                                                                       .pImmutableSamplers = nullptr};
  const vk::DescriptorSetLayoutBinding                samplerLayoutBinding{.binding = 1,
                                                                           .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                                                                           .descriptorCount = 1,
                                                                           .stageFlags = vk::ShaderStageFlagBits::eFragment,
                                                                           .pImmutableSamplers = nullptr};
  const std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
  const vk::DescriptorSetLayoutCreateInfo             layoutInfo{.bindingCount = static_cast<uint32_t>(bindings.size()),
                                                                 .pBindings    = bindings.data()};
  if (auto result = m_Device.createDescriptorSetLayout(&layoutInfo, nullptr, &m_DescriptorSetLayout);
      result != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to create descriptor set layout!");
    return false;
  }
  return true;
}
//===============================================================================
bool VulkanRenderer::CreateGraphicsPipeline()
{
  const auto verShaderCode  = ReadFile("shaders/simpleShader.vert.spv");
  const auto fragShaderCode = ReadFile("shaders/simpleShader.frag.spv");

  const auto vertShaderModule = CreateShaderModule(verShaderCode);
  const auto fragShaderModule = CreateShaderModule(fragShaderCode);

  const vk::PipelineShaderStageCreateInfo      vertShaderStageInfo{.stage  = vk::ShaderStageFlagBits::eVertex,
                                                                   .module = vertShaderModule,
                                                                   .pName  = "main"};
  const vk::PipelineShaderStageCreateInfo      fragShaderStageInfo{.stage  = vk::ShaderStageFlagBits::eFragment,
                                                                   .module = fragShaderModule,
                                                                   .pName  = "main"};
  const std::array                             shaderStages          = {vertShaderStageInfo, fragShaderStageInfo};
  const auto                                   bindingDescription    = Vertex::GetBindingDescription();
  const auto                                   attributeDescriptions = Vertex::GetAttributeDescriptions();
  const vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
    .vertexBindingDescriptionCount   = 1,
    .pVertexBindingDescriptions      = &bindingDescription,
    .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
    .pVertexAttributeDescriptions    = attributeDescriptions.data(),
  };
  const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
    .topology               = vk::PrimitiveTopology::eTriangleList,
    .primitiveRestartEnable = VK_FALSE,
  };

  const vk::PipelineViewportStateCreateInfo viewportStateInfo{.viewportCount = 1, .scissorCount = 1};

  const vk::PipelineRasterizationStateCreateInfo rasterizerInfo{
    .depthClampEnable        = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode             = vk::PolygonMode::eFill,
    .cullMode                = vk::CullModeFlagBits::eBack,
    .frontFace               = vk::FrontFace::eCounterClockwise,
    .depthBiasEnable         = VK_FALSE,
    .lineWidth               = 1.0F,
  };

  const vk::PipelineMultisampleStateCreateInfo multisamplingInfo{
    .rasterizationSamples = vk::SampleCountFlagBits::e1,
    .sampleShadingEnable  = VK_FALSE,
  };

  vk::PipelineColorBlendAttachmentState colorBlendAttachment{
    .blendEnable    = VK_FALSE,
    .colorWriteMask = (vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                       vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA),
  };

  const vk::PipelineColorBlendStateCreateInfo colorBlendingInfo{.logicOpEnable   = VK_FALSE,
                                                                .logicOp         = vk::LogicOp::eCopy,
                                                                .attachmentCount = 1,
                                                                .pAttachments    = &colorBlendAttachment,
                                                                .blendConstants  = {{0.0F, 0.0F, 0.0F, 0.0F}}};
  const std::vector                           dynamicStates = {
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor,
  };
  const vk::PipelineDynamicStateCreateInfo dynamicStateInfo{
    .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
    .pDynamicStates    = dynamicStates.data(),
  };
  const vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 1, .pSetLayouts = &m_DescriptorSetLayout};
  if (m_Device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_PipelineLayout) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("Failed to create pipeline layout");
    return false;
  }

  const vk::PipelineDepthStencilStateCreateInfo depthStencilInfo{
    .depthTestEnable       = VK_TRUE,
    .depthWriteEnable      = VK_TRUE,
    .depthCompareOp        = vk::CompareOp::eLess,
    .depthBoundsTestEnable = VK_FALSE,
    .stencilTestEnable     = VK_FALSE,
    // .front                 = {},
    // .back                  = {},
    // .minDepthBounds        = 0.0F,
    // .maxDepthBounds        = 1.0F,
  };

  const vk::GraphicsPipelineCreateInfo pipelineInfo{
    .stageCount          = 2,
    .pStages             = shaderStages.data(),
    .pVertexInputState   = &vertexInputInfo,
    .pInputAssemblyState = &inputAssemblyInfo,
    .pViewportState      = &viewportStateInfo,
    .pRasterizationState = &rasterizerInfo,
    .pMultisampleState   = &multisamplingInfo,
    .pDepthStencilState  = &depthStencilInfo,
    .pColorBlendState    = &colorBlendingInfo,
    .pDynamicState       = &dynamicStateInfo,
    .layout              = m_PipelineLayout,
    .renderPass          = m_RenderPass,
    .subpass             = 0,
    .basePipelineHandle  = VK_NULL_HANDLE,
  };

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
vk::ShaderModule VulkanRenderer::CreateShaderModule(const std::vector<char>& code)
{
  return m_Device.createShaderModule({.codeSize = code.size(), .pCode = reinterpret_cast<const uint32_t*>(code.data())});
}

//===============================================================================
vk::Format VulkanRenderer::FindDepthFormat() const
{
  return FindSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                             vk::ImageTiling::eOptimal,
                             vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

//===============================================================================
bool VulkanRenderer::HasStencilComponent(vk::Format format) const
{
  return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

//===============================================================================
bool VulkanRenderer::CreateFramebuffers()
{
  const auto& extent = GetExtent();
  m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());
  for (size_t i = 0; i < m_SwapChainImageViews.size(); ++i)
  {
    const std::array                attachments = {m_SwapChainImageViews[i], m_DepthImageView};
    const vk::FramebufferCreateInfo framebufferInfo{.renderPass      = m_RenderPass,
                                                    .attachmentCount = static_cast<uint32_t>(attachments.size()),
                                                    .pAttachments    = attachments.data(),
                                                    .width           = extent.width,
                                                    .height          = extent.height,
                                                    .layers          = 1};

    if (m_Device.createFramebuffer(&framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != vk::Result::eSuccess)
    {
      LOG_CORE_ERROR("failed to create framebuffer!");
      return false;
    }
  }
  return true;
}

//===============================================================================
bool VulkanRenderer::CreateCommandPool()
{
  const QueueFamilyIndices queueFamilyIndices{FindQueueFamilies(m_PhysicalDevice, m_Surface)};

  const vk::CommandPoolCreateInfo poolInfo{.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                           .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value()};

  if (m_Device.createCommandPool(&poolInfo, nullptr, &m_CommandPool) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("Failed to create command pool");
    return false;
  }
  if (m_Device.createCommandPool(&poolInfo, nullptr, &m_ImmediateCommandPool) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("Failed to create immediate command pool");
    return false;
  }

  // INFO: temperory here
  m_MainDeletionQueue.push_function([this] { m_Device.destroyCommandPool(m_ImmediateCommandPool); });
  return true;
}

//===============================================================================
bool VulkanRenderer::CreateCommandBuffers()
{
  m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  const vk::CommandBufferAllocateInfo allocInfo{.commandPool        = m_CommandPool,
                                                .level              = vk::CommandBufferLevel::ePrimary,
                                                .commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size())};
  const vk::CommandBufferAllocateInfo immediateallocInfo{.commandPool        = m_ImmediateCommandPool,
                                                         .level              = vk::CommandBufferLevel::ePrimary,
                                                         .commandBufferCount = 1};

  if (m_Device.allocateCommandBuffers(&allocInfo, m_CommandBuffers.data()) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("Failed to allocate command buffers");
    return false;
  }
  // INFO: temperory here
  if (m_Device.allocateCommandBuffers(&immediateallocInfo, &m_ImmediateCommandBuffer) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("Failed to allocate immediate command buffers");
    return false;
  }
  // INFO: temperory here
  m_MainDeletionQueue.push_function(
    [this] { m_Device.freeCommandBuffers(m_ImmediateCommandPool, 1, &m_ImmediateCommandBuffer); });

  return true;
}

//===============================================================================
bool VulkanRenderer::CreateSyncObjects()
{
  m_FrameData.resize(MAX_FRAMES_IN_FLIGHT);

  vk::SemaphoreCreateInfo semaphoreInfo{};
  vk::FenceCreateInfo     fenceInfo{.flags = vk::FenceCreateFlagBits::eSignaled};
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
  {
    if (m_Device.createSemaphore(&semaphoreInfo, nullptr, &m_FrameData[i].imageAvailableSemaphore) != vk::Result::eSuccess ||
        m_Device.createSemaphore(&semaphoreInfo, nullptr, &m_FrameData[i].renderFinishedSemaphore) != vk::Result::eSuccess ||
        m_Device.createFence(&fenceInfo, nullptr, &m_FrameData[i].inFlightFence) != vk::Result::eSuccess)
    {
      LOG_CORE_ERROR("Failed to create synchronization objects for a frame");
      return false;
    }
    if (m_Device.createFence(&fenceInfo, nullptr, &m_ImmediateFence) != vk::Result::eSuccess)
    {
      LOG_CORE_ERROR("Failed to create immediate fence");
      return false;
    }
    // INFO: temperory here
    m_MainDeletionQueue.push_function([this] { m_Device.destroyFence(m_ImmediateFence); });
  }
  return true;
}

//===============================================================================
bool VulkanRenderer::CreateVertexBuffers()
{
  try
  {
    const vk::DeviceSize bufferSize{sizeof(vertices[0]) * vertices.size()};

    vk::Buffer       stagingBuffer;
    vk::DeviceMemory staggingBufferMemory;
    CreateBuffer(bufferSize,
                 vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                 stagingBuffer,
                 staggingBufferMemory);
    void* data = nullptr;
    if (m_Device.mapMemory(staggingBufferMemory, 0, bufferSize, {}, &data) != vk::Result::eSuccess)
    {
      LOG_CORE_ERROR("failed to map vertex buffer memory!");
      return false;
    }
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    m_Device.unmapMemory(staggingBufferMemory);

    CreateBuffer(bufferSize,
                 vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal,
                 m_VertexBuffer,
                 m_VertexBufferMemory);

    CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);
    m_Device.destroyBuffer(stagingBuffer);
    m_Device.freeMemory(staggingBufferMemory);

    return true;
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR("failed to create vertex buffer!, exception: {0}", e.what());
  }
  return false;
}

//===============================================================================
bool VulkanRenderer::CreateIndexBuffers()
{
  try
  {
    const vk::DeviceSize bufferSize{sizeof(indices[0]) * indices.size()};
    vk::Buffer           stagingBuffer;
    vk::DeviceMemory     staggingBufferMemory;
    CreateBuffer(bufferSize,
                 vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                 stagingBuffer,
                 staggingBufferMemory);
    void* data = m_Device.mapMemory(staggingBufferMemory, 0, bufferSize);
    if (data == nullptr)
    {
      LOG_CORE_ERROR("failed to map index buffer memory!");
      return false;
    }
    memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    m_Device.unmapMemory(staggingBufferMemory);

    CreateBuffer(bufferSize,
                 vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal,
                 m_IndexBuffer,
                 m_IndexBufferMemory);

    CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);
    m_Device.destroyBuffer(stagingBuffer);
    m_Device.freeMemory(staggingBufferMemory);

    return true;
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR("failed to create index buffer!, exception: {0}", e.what());
  }
  return false;
}

//===============================================================================
bool VulkanRenderer::CreateUniformBuffers()
{
  try
  {
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

    m_UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_UniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    m_UniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
      CreateBuffer(bufferSize,
                   vk::BufferUsageFlagBits::eUniformBuffer,
                   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                   m_UniformBuffers[i],
                   m_UniformBuffersMemory[i]);
      if (m_Device.mapMemory(m_UniformBuffersMemory[i], 0, bufferSize, {}, &m_UniformBuffersMapped[i]) != vk::Result::eSuccess)
      {
        LOG_CORE_ERROR("failed to map uniform buffer memory!");
        return false;
      }
    }
    return true;
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR("failed to create uniform buffer!, exception: {0}", e.what());
  }
  return false;
}
//===============================================================================
void VulkanRenderer::ReCreateSwapChain()
{
  const auto renderTime = std::chrono::high_resolution_clock::now();
  while (m_Window.GetWidth() == 0 || m_Window.GetHeight() == 0)
  {
    m_Window.WaitEvents();
  }
  const auto renderTimeDuration = std::chrono::duration_cast<std::chrono::microseconds>(
    std::chrono::high_resolution_clock::now() - renderTime);
  LOG_CORE_INFO("rocord Render time: {0} us", renderTimeDuration.count());
  m_Device.waitIdle();
  CleanupSwapChain();
  if (CreateSwapChain() && CreateImageViews() && CreateDepthResources() && CreateFramebuffers())
  {
    // success
    return;
  }

  LOG_CORE_ERROR("failed to recreate swap chain!");
}

//===============================================================================
void VulkanRenderer::CleanupSwapChain()
{
  m_Device.destroyImageView(m_DepthImageView);
  m_Device.destroyImage(m_DepthImage);
  m_Device.freeMemory(m_DepthImageMemory);

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
void VulkanRenderer::RecordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const
{
  const auto&                      extent = GetExtent();
  const vk::CommandBufferBeginInfo beginInfo{};
  commandBuffer.begin(beginInfo);

  const std::array clearValues{
    vk::ClearValue{.color = vk::ClearColorValue{.float32 = {{0.0F, 0.0F, 0.0F, 1.0F}}}},
    vk::ClearValue{.depthStencil = {.depth = 1.0F, .stencil = 0}},
  };

  //begin render pass
  const vk::RenderPassBeginInfo renderPassInfo{
    .renderPass      = m_RenderPass,
    .framebuffer     = m_SwapChainFramebuffers[imageIndex],
    .renderArea      = vk::Rect2D{.offset = vk::Offset2D{.x = 0, .y = 0}, .extent = extent},
    .clearValueCount = static_cast<uint32_t>(clearValues.size()),
    .pClearValues    = clearValues.data(),
  };
  commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

  // basic draw
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline);
  std::array vertexBuffers = {m_VertexBuffer};
  std::array offsets       = {vk::DeviceSize{0}};

  vk::Viewport viewport{
    .x        = 0.0F,
    .y        = 0.0F,
    .width    = static_cast<float>(extent.width),
    .height   = static_cast<float>(extent.height),
    .minDepth = 0.0F,
    .maxDepth = 1.0F,
  };
  commandBuffer.setViewport(0, 1, &viewport);

  vk::Rect2D scissor{.offset = vk::Offset2D{.x = 0, .y = 0}, .extent = extent};
  commandBuffer.setScissor(0, 1, &scissor);

  commandBuffer.bindVertexBuffers(0, 1, vertexBuffers.data(), offsets.data());
  commandBuffer.bindIndexBuffer(m_IndexBuffer, 0, vk::IndexType::eUint16);
  commandBuffer
    .bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout, 0, 1, &m_DescriptorSets[m_CurrentFrame], 0, nullptr);

  commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);

  // draw ImGui
  // DrawImGui(commandBuffer, m_SwapChainImageViews[imageIndex]);

  //end render pass
  commandBuffer.endRenderPass();


  commandBuffer.end();
}

//===============================================================================
vk::CommandBuffer VulkanRenderer::BeginSingleTimeCommands() const
{
  vk::CommandBufferAllocateInfo allocInfo{.commandPool        = m_CommandPool,
                                          .level              = vk::CommandBufferLevel::ePrimary,
                                          .commandBufferCount = 1};
  vk::CommandBuffer             commandBuffer{};
  if (m_Device.allocateCommandBuffers(&allocInfo, &commandBuffer) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to allocate command buffer!");
    assert(false && "failed to allocate command buffer!");
  }
  commandBuffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  return commandBuffer;
}

//===============================================================================
void VulkanRenderer::EndSingleTimeCommands(const vk::CommandBuffer& commandBuffer) const
{
  commandBuffer.end();
  m_GraphicsQueue.submit(vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &commandBuffer});
  m_GraphicsQueue.waitIdle();
  m_Device.freeCommandBuffers(m_CommandPool, commandBuffer);
}

//===============================================================================
void VulkanRenderer::DrawFrame()
{
  [[maybe_unused]] const auto result = m_Device.waitForFences(1,
                                                              &m_FrameData[m_CurrentFrame].inFlightFence,
                                                              VK_TRUE,
                                                              std::numeric_limits<uint64_t>::max());

  GetCurrentFrameData().deletionQueue.flush();

  uint32_t imageIndex{};
  switch (const vk::Result result = m_Device.acquireNextImageKHR(m_SwapChain,
                                                                 std::numeric_limits<uint64_t>::max(),
                                                                 m_FrameData[m_CurrentFrame].imageAvailableSemaphore,
                                                                 nullptr,
                                                                 &imageIndex);
          result)
  {
    case vk::Result::eErrorOutOfDateKHR:
    {
      ReCreateSwapChain();
      return;
    }
    case vk::Result::eSuccess:
    case vk::Result::eSuboptimalKHR:
      break;
    default:
      LOG_CORE_ERROR("failed to acquire swap chain image!");
      return;
  }

  UpdateUniformBuffer(m_CurrentFrame);

  if (m_Device.resetFences(1, &m_FrameData[m_CurrentFrame].inFlightFence) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to reset fence!");
    return;
  }

  m_CommandBuffers[m_CurrentFrame].reset();
  RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);


  std::array                            waitSemaphores   = {m_FrameData[m_CurrentFrame].imageAvailableSemaphore};
  std::array<vk::PipelineStageFlags, 1> waitStages       = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
  std::array                            signalSemaphores = {m_FrameData[m_CurrentFrame].renderFinishedSemaphore};

  assert(m_CommandBuffers[m_CurrentFrame] && "Command buffer is null!");
  m_GraphicsQueue.submit(vk::SubmitInfo{.waitSemaphoreCount   = 1,
                                        .pWaitSemaphores      = waitSemaphores.data(),
                                        .pWaitDstStageMask    = waitStages.data(),
                                        .commandBufferCount   = 1,
                                        .pCommandBuffers      = &m_CommandBuffers[m_CurrentFrame],
                                        .signalSemaphoreCount = 1,
                                        .pSignalSemaphores    = signalSemaphores.data()},
                         m_FrameData[m_CurrentFrame].inFlightFence);

  std::array swapChains = {m_SwapChain};
  if (const vk::Result result = m_PresentQueue.presentKHR(
        {.waitSemaphoreCount = 1,
         .pWaitSemaphores    = signalSemaphores.data(),
         .swapchainCount     = 1,
         .pSwapchains        = swapChains.data(),
         .pImageIndices      = &imageIndex});
      result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_Window.WasWindowResized())
  {
    m_Window.ResetWindowResized();
    ReCreateSwapChain();
  }
  else if (result != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to present swap chain image!");
  }

  m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

//===============================================================================
void VulkanRenderer::DrawImGui(vk::CommandBuffer cmd, vk::ImageView targetImageView) const
{
  vk::RenderingAttachmentInfo colorAttachment{.imageView   = targetImageView,
                                              .imageLayout = vk::ImageLayout::eColorAttachmentOptimal};
  cmd.beginRendering({
    .layerCount           = 1,               //
    .colorAttachmentCount = 1,               //
    .pColorAttachments    = &colorAttachment //
  });
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
  cmd.endRendering();
}

//===============================================================================
void VulkanRenderer::CreateBuffer(vk::DeviceSize          size,
                                  vk::BufferUsageFlags    usage,
                                  vk::MemoryPropertyFlags properties,
                                  vk::Buffer&             buffer,
                                  vk::DeviceMemory&       bufferMemory)
{
  buffer = m_Device.createBuffer(
    vk::BufferCreateInfo{.size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive});

  const vk::MemoryRequirements memRequirment = m_Device.getBufferMemoryRequirements(buffer);
  const vk::MemoryAllocateInfo allocInfo{.allocationSize  = memRequirment.size,
                                         .memoryTypeIndex = FindMemoryType(memRequirment.memoryTypeBits, properties)};
  if (m_Device.allocateMemory(&allocInfo, nullptr, &bufferMemory) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to allocate buffer memory!");
    throw std::runtime_error("failed to allocate buffer memory!");
  }
  m_Device.bindBufferMemory(buffer, bufferMemory, 0);
}

//===============================================================================
bool VulkanRenderer::CreateDescriptorPool()
{
  const std::array<vk::DescriptorPoolSize, 2> poolSizes{
    {{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)},
     {.type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)}},
  };
  const vk::DescriptorPoolCreateInfo poolInfo{.maxSets       = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
                                              .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
                                              .pPoolSizes    = poolSizes.data()};
  if (m_Device.createDescriptorPool(&poolInfo, nullptr, &m_DescriptorPool) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to create descriptor pool!");
    return false;
  }
  return true;
}

//===============================================================================
bool VulkanRenderer::CreateDescriptorSets()
{
  std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);
  vk::DescriptorSetAllocateInfo        allocInfo{
           .descriptorPool     = m_DescriptorPool,
           .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
           .pSetLayouts        = layouts.data(),
  };
  m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
  if (m_Device.allocateDescriptorSets(&allocInfo, m_DescriptorSets.data()) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to allocate descriptor sets!");
    return false;
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
  {
    const vk::DescriptorBufferInfo bufferInfo{
      .buffer = m_UniformBuffers[i],
      .offset = 0,
      .range  = sizeof(UniformBufferObject),
    };
    const vk::DescriptorImageInfo imageInfo{
      .sampler     = m_TextureSampler,
      .imageView   = m_TextureImageView,
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
    };
    const std::array<vk::WriteDescriptorSet, 2> writeDescriptorSets{{
      {
        .dstSet          = m_DescriptorSets[i],
        .dstBinding      = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType  = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo     = &bufferInfo,
      },
      {
        .dstSet          = m_DescriptorSets[i],
        .dstBinding      = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType  = vk::DescriptorType::eCombinedImageSampler,
        .pImageInfo      = &imageInfo,
      },
    }};
    m_Device.updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
  }
  return true;
}

//===============================================================================
bool VulkanRenderer::CreateDepthResources()
{
  try
  {
    const vk::Format depthFormat = FindDepthFormat();
    CreateImage(m_SwapChainExtent.width,
                m_SwapChainExtent.height,
                depthFormat,
                vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eDepthStencilAttachment,
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                m_DepthImage,
                m_DepthImageMemory);
    m_DepthImageView = CreateImageView(m_DepthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
    return true;
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR("failed to create depth resources!");
  }
  return false;
}

//===============================================================================
bool VulkanRenderer::CreateTextureImage()
{
  try
  {
    int   texWidth{0};
    int   texHeight{0};
    int   texChannels{0};
    auto* pixels = stbi_load("assets/statue.jpg", &texWidth, &texHeight, &texChannels, 4); // load image and read pixels
    vk::DeviceSize imageSize = texWidth * texHeight * 4;

    if (pixels == nullptr)
    {
      LOG_CORE_ERROR("failed to load texture image!");
      return false;
    }
    vk::Buffer       stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    CreateBuffer(imageSize,
                 vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                 stagingBuffer,
                 stagingBufferMemory);
    auto* data = m_Device.mapMemory(stagingBufferMemory, 0, imageSize);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    m_Device.unmapMemory(stagingBufferMemory);

    stbi_image_free(pixels);

    CreateImage(static_cast<uint32_t>(texWidth),
                static_cast<uint32_t>(texHeight),
                vk::Format::eR8G8B8A8Srgb,
                vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                m_TextureImage,
                m_TextureImageMemory);
    TransitionImageLayout(m_TextureImage,
                          vk::Format::eR8G8B8A8Srgb,
                          vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eTransferDstOptimal);
    CopyBufferToImage(stagingBuffer, m_TextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    TransitionImageLayout(m_TextureImage,
                          vk::Format::eR8G8B8A8Srgb,
                          vk::ImageLayout::eTransferDstOptimal,
                          vk::ImageLayout::eShaderReadOnlyOptimal);

    // clean up
    m_Device.destroyBuffer(stagingBuffer);
    m_Device.freeMemory(stagingBufferMemory);

    return true;
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR(e.what());
  }
  return false;
}

//===============================================================================
bool VulkanRenderer::CreateTextureImageView()
{
  try
  {
    m_TextureImageView = CreateImageView(m_TextureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
    return true;
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR("failed to create texture image view. exception: {}", e.what());
  }
  return false;
}

//===============================================================================
bool VulkanRenderer::CreateTextureSampler()
{
  try
  {
    m_TextureSampler = m_Device.createSampler({
      .magFilter               = vk::Filter::eLinear,
      .minFilter               = vk::Filter::eLinear,
      .mipmapMode              = vk::SamplerMipmapMode::eLinear,
      .addressModeU            = vk::SamplerAddressMode::eRepeat,
      .addressModeV            = vk::SamplerAddressMode::eRepeat,
      .addressModeW            = vk::SamplerAddressMode::eRepeat,
      .mipLodBias              = 0.0F,
      .anisotropyEnable        = VK_FALSE,
      .maxAnisotropy           = 1.0F,
      .borderColor             = vk::BorderColor::eIntOpaqueBlack,
      .unnormalizedCoordinates = VK_FALSE,
      .compareEnable           = VK_FALSE,
      .compareOp               = vk::CompareOp::eAlways,
      .minLod                  = 0.0F,
      .maxLod                  = 0.0F,
    });
    return true;
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR("failed to create texture sampler. exception: {}", e.what());
  }
  return false;
}
//===============================================================================
void VulkanRenderer::CreateImage(
  uint32_t                width,
  uint32_t                height,
  vk::Format              format,
  vk::ImageTiling         tiling,
  vk::ImageUsageFlags     usage,
  vk::MemoryPropertyFlags properties,
  vk::Image&              textureImage,
  vk::DeviceMemory&       textureImageMemory) const
{
  textureImage              = m_Device.createImage({
                 .imageType     = vk::ImageType::e2D,
                 .format        = format,
                 .extent        = {.width = width, .height = height, .depth = 1},
                 .mipLevels     = 1,
                 .arrayLayers   = 1,
                 .samples       = vk::SampleCountFlagBits::e1,
                 .tiling        = tiling,
                 .usage         = usage,
                 .sharingMode   = vk::SharingMode::eExclusive,
                 .initialLayout = vk::ImageLayout::eUndefined,
  });
  const auto memRequirments = m_Device.getImageMemoryRequirements(textureImage);
  textureImageMemory        = m_Device.allocateMemory({
           .allocationSize  = memRequirments.size,
           .memoryTypeIndex = FindMemoryType(memRequirments.memoryTypeBits, properties),
  });
  m_Device.bindImageMemory(textureImage, textureImageMemory, 0);
}

//===============================================================================
void VulkanRenderer::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) const
{
  vk::CommandBuffer commandBuffer{BeginSingleTimeCommands()};
  vk::BufferCopy    copyRegion{.size = size};
  commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);
  EndSingleTimeCommands(commandBuffer);
}

//===============================================================================
void VulkanRenderer::StopRenderImpl()
{
  m_Device.waitIdle();
}

//===============================================================================
uint32_t VulkanRenderer::FindMemoryType(uint32_t typeFilter, const vk::MemoryPropertyFlags& properties) const
{
  vk::PhysicalDeviceMemoryProperties memProperties{m_PhysicalDevice.getMemoryProperties()};
  for (size_t i = 0; i < memProperties.memoryTypeCount; ++i)
  {
    if (((typeFilter & (1 << i)) != 0U) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
    {
      return i;
    }
  }

  LOG_CORE_ERROR("failed to find suitable memory type!");
  // TODO: not sure if it is recoverable
  assert(false && "failed to find suitable memory type!");
  return {};
}

//===============================================================================
void VulkanRenderer::UpdateUniformBuffer(uint32_t currentImage)
{
  static auto startTime = std::chrono::high_resolution_clock::now();
  m_MainCamera.Update(0.0F);

  const auto  currentTime = std::chrono::high_resolution_clock::now();
  const float time        = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
  glm::mat4   view        = m_MainCamera.GetViewMatrix();

  UniformBufferObject
    ubo{.model = glm::rotate(glm::mat4(1.0F), time * glm::radians(90.0F), glm::vec3(0.0F, 0.0F, 1.0F)),
        .view = m_MainCamera.GetViewMatrix(), //glm::lookAt(glm::vec3(2.0F, 2.0F, 2.0F), glm::vec3(0.0F, 0.0F, 0.0F), glm::vec3(0.0F, 0.0F, 1.0F)),
        .proj = glm::perspective(glm::radians(45.0F),
                                 static_cast<float>(m_SwapChainExtent.width) / static_cast<float>(m_SwapChainExtent.height),
                                 0.1F,
                                 10.F)};
  ubo.proj[1][1] *= -1;
  memcpy(m_UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

//===============================================================================
void VulkanRenderer::TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const
{
  const vk::CommandBuffer commandBuffer{BeginSingleTimeCommands()};

  vk::PipelineStageFlags sourceStage{};
  vk::PipelineStageFlags destinationStage{};
  vk::ImageMemoryBarrier barrier{
    .oldLayout        = oldLayout,
    .newLayout        = newLayout,
    .image            = image,
    .subresourceRange = {.aspectMask     = vk::ImageAspectFlagBits::eColor,
                         .baseMipLevel   = 0,
                         .levelCount     = 1,
                         .baseArrayLayer = 0,
                         .layerCount     = 1},
  };

  if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
  {
    barrier.srcAccessMask = {};
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

    sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
    destinationStage = vk::PipelineStageFlagBits::eTransfer;
  }
  else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
  {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    sourceStage      = vk::PipelineStageFlagBits::eTransfer;
    destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
  }
  else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eStencilAttachmentOptimal)
  {
    barrier.srcAccessMask = {};
    barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

    sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
    destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
  }
  else
  {
    LOG_CORE_ERROR("unsupported layout transition!");
    return;
  }

  commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);

  EndSingleTimeCommands(commandBuffer);
}

//===============================================================================
void VulkanRenderer::CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) const
{
  const vk::CommandBuffer commandBuffer{BeginSingleTimeCommands()};

  vk::BufferImageCopy region{
    .bufferOffset      = 0,
    .bufferRowLength   = 0,
    .bufferImageHeight = 0,
    .imageSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
    .imageOffset = {.x = 0, .y = 0, .z = 0},
    .imageExtent = {.width = width, .height = height, .depth = 1},
  };

  commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

  EndSingleTimeCommands(commandBuffer);
}

//========================================================================
void VulkanRenderer::CopyImageToImage(vk::CommandBuffer cmd,
                                      vk::Image         srcImage,
                                      vk::Image         dstImage,
                                      vk::Extent2D      srcSize,
                                      vk::Extent2D      dstSize) const
{
  vk::ImageBlit2 blitRegion{.sType = vk::StructureType::eImageBlit2, .pNext = nullptr};

  blitRegion.srcOffsets[1].x = srcSize.width;
  blitRegion.srcOffsets[1].y = srcSize.height;
  blitRegion.srcOffsets[1].z = 1;

  blitRegion.dstOffsets[1].x = dstSize.width;
  blitRegion.dstOffsets[1].y = dstSize.height;
  blitRegion.dstOffsets[1].z = 1;

  blitRegion.srcSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
  blitRegion.srcSubresource.baseArrayLayer = 0;
  blitRegion.srcSubresource.layerCount     = 1;
  blitRegion.srcSubresource.mipLevel       = 0;

  blitRegion.dstSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
  blitRegion.dstSubresource.baseArrayLayer = 0;
  blitRegion.dstSubresource.layerCount     = 1;
  blitRegion.dstSubresource.mipLevel       = 0;

  vk::BlitImageInfo2 blitInfo{.sType = vk::StructureType::eBlitImageInfo2, .pNext = nullptr};
  blitInfo.dstImage       = dstImage;
  blitInfo.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
  blitInfo.srcImage       = srcImage;
  blitInfo.srcImageLayout = vk::ImageLayout::eTransferSrcOptimal;
  blitInfo.filter         = vk::Filter::eLinear;
  blitInfo.regionCount    = 1;
  blitInfo.pRegions       = &blitRegion;

  cmd.blitImage2(&blitInfo);
}

//========================================================================
vk::ImageView VulkanRenderer::CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspect) const
{
  return m_Device.createImageView({
    .image    = image,
    .viewType = vk::ImageViewType::e2D,
    .format   = format,
    .subresourceRange =
      {
        .aspectMask     = aspect,
        .baseMipLevel   = 0,
        .levelCount     = 1,
        .baseArrayLayer = 0,
        .layerCount     = 1,
      },
  });
}
bool VulkanRenderer::InitImGui()
{
  try
  {
    // 1: create descriptor pool for IMGUI
    //  the size of the pool is very oversize, but it's copied from imgui demo
    //  itself.
    std::array<vk::DescriptorPoolSize, 11> pool_sizes =
      {vk::DescriptorPoolSize{.type = vk::DescriptorType::eSampler, .descriptorCount = 1000},
       {.type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = 1000},
       {.type = vk::DescriptorType::eSampledImage, .descriptorCount = 1000},
       {.type = vk::DescriptorType::eStorageImage, .descriptorCount = 1000},
       {.type = vk::DescriptorType::eUniformTexelBuffer, .descriptorCount = 1000},
       {.type = vk::DescriptorType::eStorageTexelBuffer, .descriptorCount = 1000},
       {.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1000},
       {.type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1000},
       {.type = vk::DescriptorType::eUniformBufferDynamic, .descriptorCount = 1000},
       {.type = vk::DescriptorType::eStorageBufferDynamic, .descriptorCount = 1000},
       {.type = vk::DescriptorType::eInputAttachment, .descriptorCount = 1000}};

    vk::DescriptorPoolCreateInfo pool_info = {.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                              .maxSets       = 1000,
                                              .poolSizeCount = static_cast<u32>(std::size(pool_sizes)),
                                              .pPoolSizes    = pool_sizes.data()};

    vk::DescriptorPool imguiPool = m_Device.createDescriptorPool(pool_info);

    // 2: initialize imgui library

    // this initializes the core structures of imgui
    ImGui::CreateContext();

    // this initializes imgui for SDL
    if (!ImGui_ImplGlfw_InitForVulkan(m_Window.GetHandle(), false))
    {
      LOG_CORE_ERROR("ImGui_ImplGlfw_InitForVulkan failed.");
      return false;
    }

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = m_Instance;
    init_info.PhysicalDevice            = m_PhysicalDevice;
    init_info.Device                    = m_Device;
    init_info.Queue                     = m_GraphicsQueue;
    init_info.DescriptorPool            = imguiPool;
    init_info.MinImageCount             = 3;
    init_info.ImageCount                = 3;
    init_info.UseDynamicRendering       = true;

    //dynamic rendering parameters for imgui to use
    const auto swapChainImageFormat       = (VkFormat)m_SwapChainImageFormat;
    init_info.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    init_info.PipelineRenderingCreateInfo.colorAttachmentCount    = 1;
    init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapChainImageFormat;


    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info);

    ImGui_ImplVulkan_CreateFontsTexture();

    // add the destroy the imgui created structures
    m_MainDeletionQueue.push_function(
      [imguiPool, this]()
      {
        ImGui_ImplVulkan_Shutdown();
        m_Device.destroyDescriptorPool(imguiPool);
      });
    return true;
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR("failed to initialize imgui. exception: {}", e.what());
  }
  return false;
}
//========================================================================
} // namespace four
