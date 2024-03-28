#pragma once

#include "vulkan/vulkan.hpp"
#include "core/core.hpp"

namespace four
{

//===========================================================================================
struct SwapChainSupportDetails
{
  vk::SurfaceCapabilitiesKHR        capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR>   presentModes;
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

  // enable validation layer for debugging in debug mode
#ifdef NDEBUG
  static constexpr bool enableValidationLayers = false;
#else
  static constexpr bool enableValidationLayers = true;
#endif

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
  [[nodiscard]] inline vk::CommandPool GetCommandPool() const noexcept
  {
    return m_CommandPool;
  }

  //===========================================================================================
  [[nodiscard]] inline vk::Device GetDevice() const noexcept
  {
    return m_Device;
  }

  //===========================================================================================
  [[nodiscard]] inline vk::SurfaceKHR GetSurface() const noexcept
  {
    return m_Surface;
  }

  //===========================================================================================
  [[nodiscard]] inline vk::Queue GetGraphicsQueue() const noexcept
  {
    return m_GraphicsQueue;
  }

  //===========================================================================================
  [[nodiscard]] inline vk::Queue GetPresentQueue() const noexcept
  {
    return m_PresentQueue;
  }

  //===========================================================================================
  [[nodiscard]] inline vk::PhysicalDevice GetPhysicalDevice() const noexcept
  {
    return m_PhysicalDevice;
  }

  //===========================================================================================
  [[nodiscard]] inline SwapChainSupportDetails GetSwapChainSupport()
  {
    return QuerySwapChainSupport(m_PhysicalDevice);
  }

  //===========================================================================================
  [[nodiscard]] uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

  //===========================================================================================
  [[nodiscard]] inline QueueFamilyIndices FindPhysicalQueueFamilies()
  {
    return FindQueueFamilies(m_PhysicalDevice);
  }

  //===========================================================================================
  [[nodiscard]] inline vk::Instance* GetInstance()
  {
    return &m_Instance;
  }

  //===========================================================================================
  [[nodiscard]] vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates,
                                               vk::ImageTiling                tiling,
                                               vk::FormatFeatureFlags         features);

  // Buffer Helper Functions
  //===========================================================================================
  void CreateBuffer(vk::DeviceSize          size,
                    vk::BufferUsageFlags    usage,
                    vk::MemoryPropertyFlags properties,
                    vk::Buffer&             buffer,
                    vk::DeviceMemory&       bufferMemory);

  vk::CommandBuffer BeginSingleTimeCommands();

  void EndSingleTimeCommands(vk::CommandBuffer commandBuffer);

  void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

  void CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount);

  void CreateImageWithInfo(const vk::ImageCreateInfo& imageInfo,
                           vk::MemoryPropertyFlags    properties,
                           vk::Image&                 image,
                           vk::DeviceMemory&          imageMemory);

  vk::PhysicalDeviceProperties properties;

private:
  void CreateInstance();

  void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

  void SetupDebugMessenger();

  std::vector<const char*> GetRequiredExtensions();

  bool CheckValidationLayerSupport();

  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                      void*                                       pUserData)
  {
    LOG_CORE_ERROR("validation layer: {}", pCallbackData->pMessageIdName);

    return VK_FALSE;
  }

  void CreateSurface();

  void PickPhysicalDevice();

  void CreateLogicalDevice();

  void CreateCommandPool();

  // helper functions
  [[nodiscard]] bool IsDeviceSuitable(vk::PhysicalDevice device);

  [[nodiscard]] QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice device);

  void HasGflwRequiredInstanceExtensions();

  [[nodiscard]] bool CheckDeviceExtensionSupport(vk::PhysicalDevice device);

  [[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice device);

  void CreateWindowSurface(vk::Instance instance, vk::SurfaceKHR* surface);

private:
  Window<GlfwWindow>*      m_Window{nullptr};
  vk::Instance             m_Instance;
  VkDebugUtilsMessengerEXT m_DebugMessenger;

  vk::PhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
  vk::CommandPool    m_CommandPool;

  vk::Device     m_Device;
  vk::SurfaceKHR m_Surface;
  vk::Queue      m_GraphicsQueue;
  vk::Queue      m_PresentQueue;

  const std::vector<const char*> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char*> m_DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

} // namespace four
