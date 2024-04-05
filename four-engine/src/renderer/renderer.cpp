#include "four-pch.h"

#include "renderer/renderer.hpp"

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
  InitVulkan();
  return true;
}

//===============================================================================
void Renderer::Shutdown()
{
  if (m_Instance)
  {
    m_Instance.destroy();
    LOG_CORE_INFO("Vulkan uninitialized");
  }
}

//===============================================================================
bool Renderer::InitVulkan()
{
  CreateInstance();
  SetupDebugMessenger();
  LOG_CORE_INFO("Vulkan initialized");
  return true;
}

//===============================================================================
void Renderer::CreateInstance()
{
  if (EnableValidationLayers && !CheckValidationLayerSupport())
  {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  vk::ApplicationInfo    appInfo{"four-engine", 1, "four-engine", 1, VK_API_VERSION_1_3};
  vk::InstanceCreateInfo createInfo{vk::InstanceCreateFlags(), &appInfo};

  auto extensions                    = GetRequiredExtensions();
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
  }

  m_Instance = vk::createInstance(createInfo);
  if (!m_Instance)
  {
    throw std::runtime_error("failed to create instance!");
  }
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

std::vector<const char*> Renderer::GetRequiredExtensions()
{
  std::vector<const char*> extensions = m_Window.GetVulkanRequiredExtensions();

  if (EnableValidationLayers)
  {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  return extensions;
}

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
} // namespace four
