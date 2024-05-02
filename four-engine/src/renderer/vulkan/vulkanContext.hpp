#pragma once

#include "core/core.hpp"

#include "vulkan/vulkan.hpp"

namespace four
{

class GlfwWindow;
template <typename T>
class Window;

#ifdef NDEBUG
constexpr bool EnableValidationLayers = false;
#else
constexpr bool EnableValidationLayers = true;
#endif

const std::array<const char*, 1> ValidationLayers = {"VK_LAYER_KHRONOS_validation"};

class FOUR_ENGINE_API VulkanContext
{
public:
  struct QueueFamilyIndices
  {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool IsComplete() const
    {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  struct SwapChainSupportDetails
  {
    vk::SurfaceCapabilitiesKHR        capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR>   presentModes;
  };
  explicit VulkanContext(Window<GlfwWindow>& window);
  ~VulkanContext() = default;

  VulkanContext(const VulkanContext&)            = delete;
  VulkanContext(VulkanContext&&)                 = delete;
  VulkanContext& operator=(const VulkanContext&) = delete;
  VulkanContext& operator=(VulkanContext&&)      = delete;

  bool Init();
  void Shutdown();

  [[nodiscard]] static SwapChainSupportDetails QuerySwapChainSupport(const vk::PhysicalDevice& device,
                                                                     const vk::SurfaceKHR&     surface);
  [[nodiscard]] static QueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);

  [[nodiscard]] inline const vk::Instance& GetVulkanInstance() const
  {
    return m_Instance;
  }
  [[nodiscard]] inline const vk::PhysicalDevice& GetPhysicalDevice() const
  {
    return m_PhysicalDevice;
  }
  [[nodiscard]] inline const vk::SurfaceKHR& GetSurface() const
  {
    return m_Surface;
  }
  [[nodiscard]] inline const std::vector<const char*>& GetDeviceExtensions() const
  {
    return m_DeviceExtensions;
  }

  [[nodiscard]] const vk::Device& GetDevice() const
  {
    return m_Device;
  }

  [[nodiscard]] vk::Extent2D GetExtent() const
  {
    return m_SwapChainExtent;
  }

private:
  [[nodiscard]] bool CreateInstance();
  [[nodiscard]] bool SetupDebugMessenger();
  [[nodiscard]] bool CreateSurface();
  [[nodiscard]] bool PickPhysicalDevice();
  [[nodiscard]] bool CreateLogicalDevice();

  // helpers
  [[nodiscard]] static bool              CheckValidationLayerSupport();
  [[nodiscard]] std::vector<const char*> GetRequiredExtensions();
  [[nodiscard]] bool                     IsDeviceSuitable(const vk::PhysicalDevice& device) const;
  [[nodiscard]] bool                     CheckDeviceExtensionSupport(const vk::PhysicalDevice& device) const;
  [[nodiscard]] vk::Extent2D             ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

  static void                                         PrintExtensionsSupport();
  [[nodiscard]] static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSensivity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/);

  [[nodiscard]] uint32_t RateDeviceSuitability(const vk::PhysicalDevice& device) const;

private:
  Window<GlfwWindow>&        m_Window;
  vk::Instance               m_Instance;
  vk::DebugUtilsMessengerEXT m_DebugMessenger;
  vk::SurfaceKHR             m_Surface;
  vk::PhysicalDevice         m_PhysicalDevice;
  vk::Device                 m_Device;
  vk::Queue                  m_GraphicsQueue;
  vk::Queue                  m_PresentQueue;
  std::vector<const char*>   m_DeviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  vk::Extent2D               m_SwapChainExtent;
};
} // namespace four
