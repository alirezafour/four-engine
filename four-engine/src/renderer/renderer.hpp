#pragma once

#include "core/core.hpp"

#include <vulkan/vulkan.hpp>

namespace four
{

class GlfwWindow;
template <typename T>
class Window;

const std::array<const char*, 1> ValidationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool EnableValidationLayers = false;
#else
constexpr bool EnableValidationLayers = true;
#endif

class Renderer
{
  struct QueueFamilyIndices
  {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool IsComplete() const
    {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

public:
  explicit Renderer(Window<GlfwWindow>& window);
  ~Renderer();

  Renderer(const Renderer&)                = delete;
  Renderer& operator=(const Renderer&)     = delete;
  Renderer(Renderer&&) noexcept            = delete;
  Renderer& operator=(Renderer&&) noexcept = delete;

  bool Init();
  void Shutdown();

private:
  bool InitVulkan();
  void CreateVulkanInstance();
  void SetupDebugMessenger();
  void PickPhysicalDevice();
  void CreateLogicalDevice();
  void CreateSurface();

  std::vector<const char*> GetRequiredExtensions();

  static void                                         PrintExtensionsSupport();
  [[nodiscard]] static bool                           CheckValidationLayerSupport();
  [[nodiscard]] static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSensivity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/);
  [[nodiscard]] bool               IsDeviceSuitable(const vk::PhysicalDevice& device);
  [[nodiscard]] bool               CheckDeviceExtensionSupport(const vk::PhysicalDevice& device);
  [[nodiscard]] int                RateDeviceSuitability(const vk::PhysicalDevice& device);
  [[nodiscard]] QueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice& device);

private:
  Window<GlfwWindow>&        m_Window;
  vk::Instance               m_Instance;
  vk::DebugUtilsMessengerEXT m_DebugMessenger;
  vk::PhysicalDevice         m_PhysicalDevice;
  vk::Device                 m_Device;
  vk::Queue                  m_GraphicsQueue;
  vk::SurfaceKHR             m_Surface;
  vk::Queue                  m_PresentQueue;
  std::vector<const char*>   m_DeviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};
} // namespace four
