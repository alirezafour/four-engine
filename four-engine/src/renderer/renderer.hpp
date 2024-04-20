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

  struct SwapChainSupportDetails
  {
    vk::SurfaceCapabilitiesKHR        capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR>   presentModes;
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
  [[nodiscard]] bool InitVulkan();
  [[nodiscard]] bool CreateVulkanInstance();
  [[nodiscard]] bool SetupDebugMessenger();
  [[nodiscard]] bool PickPhysicalDevice();
  [[nodiscard]] bool CreateLogicalDevice();
  [[nodiscard]] bool CreateSurface();
  [[nodiscard]] bool CreateSwapChain();
  [[nodiscard]] bool CreateImageView();

  [[nodiscard]] std::vector<const char*> GetRequiredExtensions() const;

  static void                                         PrintExtensionsSupport();
  [[nodiscard]] static bool                           CheckValidationLayerSupport();
  [[nodiscard]] static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSensivity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/);

  [[nodiscard]] bool                    IsDeviceSuitable(const vk::PhysicalDevice& device) const;
  [[nodiscard]] bool                    CheckDeviceExtensionSupport(const vk::PhysicalDevice& device) const;
  [[nodiscard]] int                     RateDeviceSuitability(const vk::PhysicalDevice& device) const;
  [[nodiscard]] QueueFamilyIndices      FindQueueFamilies(const vk::PhysicalDevice& device) const;
  [[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport(const vk::PhysicalDevice& device) const;
  [[nodiscard]] static vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
  [[nodiscard]] static vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
  [[nodiscard]] vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);


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
  vk::SwapchainKHR           m_SwapChain;
  std::vector<vk::Image>     m_SwapChainImages;
  vk::Format                 m_SwapChainImageFormat;
  vk::Extent2D               m_SwapChainExtent;
  std::vector<vk::ImageView> m_SwapChainImageViews;
};
} // namespace four
