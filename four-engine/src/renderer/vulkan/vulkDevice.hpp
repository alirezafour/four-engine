#pragma once

#include "core/core.hpp"
#include <vulkan/vulkan.h>


namespace four
{

const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

// TODO:
// INFO: Testing
//
// #ifdef NDEBUG
// const bool enableValidationLayers = false;
// #else
// const bool enableValidationLayers = true;
// #endif
const bool enableValidationLayers = false;

//===========================================================================================
inline VkResult createDebugUtilsMessengerEXT(VkInstance                                instance,
                                             const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks*              pAllocator,
                                             VkDebugUtilsMessengerEXT*                 pCallback)
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr)
  {
    return func(instance, pCreateInfo, pAllocator, pCallback);
  }
  else
  {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

//===========================================================================================
inline void DestroyDebugUtilsMessengerEXT(VkInstance                   instance,
                                          VkDebugUtilsMessengerEXT     callback,
                                          const VkAllocationCallbacks* pAllocator)
{
  if (auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
                                                                             "vkDestroyDebugUtilsMessengerEXT");
      func != nullptr)
  {
    func(instance, callback, pAllocator);
  }
}

//===========================================================================================
struct SwapChainSupportDetails
{
  VkSurfaceCapabilitiesKHR        capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR>   presentModes;
};

//===========================================================================================
struct QueueFamilyIndices
{
  uint32_t graphicsFamily{};
  uint32_t presentFamily{};
  bool     graphicsFamilyHasValue = false;
  bool     presentFamilyHasValue  = false;

  [[nodiscard]] bool IsComplete() const
  {
    return graphicsFamilyHasValue && presentFamilyHasValue;
  }
};

// forward declare
template <typename T>
class Window;
class GlfwWindow;

class VulkDevice
{
public:
  VulkDevice() = default;
  explicit VulkDevice(Window<GlfwWindow>* window);
  VulkDevice(const VulkDevice&)  = delete;
  VulkDevice(const VulkDevice&&) = delete;
  void operator=(VulkDevice&)    = delete;
  void operator=(VulkDevice&&)   = delete;

  void InitVulkan();

  void Cleanup();

  //===========================================================================================
  [[nodiscard]] inline VkCommandPool getCommandPool() const
  {
    return m_CommandPool;
  }

  //===========================================================================================
  [[nodiscard]] inline VkDevice device() const
  {
    return m_Device;
  }

  //===========================================================================================
  [[nodiscard]] inline VkSurfaceKHR surface() const
  {
    return m_Surface;
  }

  //===========================================================================================
  [[nodiscard]] inline VkQueue graphicsQueue() const
  {
    return m_GraphicsQueue;
  }

  //===========================================================================================
  [[nodiscard]] inline VkQueue presentQueue() const
  {
    return m_PresentQueue;
  }

  //===========================================================================================
  [[nodiscard]] inline SwapChainSupportDetails GetSwapChainSupport()
  {
    return QuerySwapChainSupport(m_PhysicalDevice);
  }

  uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

  //===========================================================================================
  [[nodiscard]] inline QueueFamilyIndices findPhysicalQueueFamilies()
  {
    return FindQueueFamilies(m_PhysicalDevice);
  }

  //===========================================================================================
  VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

  // Buffer Helper Functions
  //===========================================================================================
  void CreateBuffer(VkDeviceSize          size,
                    VkBufferUsageFlags    usage,
                    VkMemoryPropertyFlags properties,
                    VkBuffer&             buffer,
                    VkDeviceMemory&       bufferMemory);

  VkCommandBuffer BeginSingleTimeCommands();

  void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

  void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

  void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

  void CreateImageWithInfo(const VkImageCreateInfo& imageInfo,
                           VkMemoryPropertyFlags    properties,
                           VkImage&                 image,
                           VkDeviceMemory&          imageMemory);

  VkPhysicalDeviceProperties properties;

private:
  void CreateInstance();

  //===========================================================================================
  void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

  void SetupDebugMessenger();

  std::vector<const char*> GetRequiredExtensions();

  bool CheckValidationLayerSupport();

  //===========================================================================================
  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                      void*                                       pUserData)
  {
    LOG_CORE_ERROR("validation layer: {}\n", pCallbackData->pMessage);

    return VK_FALSE;
  }

  void CreateSurface();

  void PickPhysicalDevice();

  void CreateLogicalDevice();

  void CreateCommandPool();

  // helper functions
  bool IsDeviceSuitable(VkPhysicalDevice device);

  std::vector<const char*> getRequiredExtensions();

  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

  void HasGflwRequiredInstanceExtensions();

  bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

  SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

  void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
  Window<GlfwWindow>*      m_Window;
  VkInstance               m_Instance;
  VkDebugUtilsMessengerEXT m_DebugMessenger;

  VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
  VkCommandPool    m_CommandPool;

  VkDevice     m_Device;
  VkSurfaceKHR m_Surface;
  VkQueue      m_GraphicsQueue;
  VkQueue      m_PresentQueue;

  const std::vector<const char*> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char*> m_DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

} // namespace four