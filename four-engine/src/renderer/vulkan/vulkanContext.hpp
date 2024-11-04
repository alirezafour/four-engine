#pragma once

#include "core/core.hpp"

#include "vulkan/vulkan.hpp"

namespace four
{

class GlfwWindow;
template <typename T>
class Window;

#ifdef NDEBUG
constexpr bool EnableValidationLayers = true;
#else
constexpr bool EnableValidationLayers = false;
#endif

const std::array<const char*, 1> ValidationLayers = {"VK_LAYER_KHRONOS_validation"};

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

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
  ~VulkanContext();

  VulkanContext(const VulkanContext&)            = delete;
  VulkanContext(VulkanContext&&)                 = delete;
  VulkanContext& operator=(const VulkanContext&) = delete;
  VulkanContext& operator=(VulkanContext&&)      = delete;


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

  [[nodiscard]] vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates,
                                               vk::ImageTiling                tiling,
                                               vk::FormatFeatureFlags         features) const;

  void DrawFrame();

  void StopRender();

private:
  [[nodiscard]] bool Init();
  void               Shutdown();

  [[nodiscard]] bool CreateInstance();
  [[nodiscard]] bool SetupDebugMessenger();
  [[nodiscard]] bool CreateSurface();
  [[nodiscard]] bool PickPhysicalDevice();
  [[nodiscard]] bool CreateLogicalDevice();

  [[nodiscard]] bool CreateSwapChain();
  [[nodiscard]] bool CreateImageViews();

  // graphic pipeline
  [[nodiscard]] bool             CreateRenderPass();
  [[nodiscard]] bool             CreateGraphicsPipeline();
  [[nodiscard]] vk::ShaderModule CreateShaderModule(const std::vector<char>& code);
  [[nodiscard]] vk::Format       FindDepthFormat() const;

  [[nodiscard]] bool CreateFramebuffers();
  [[nodiscard]] bool CreateCommandPool();
  [[nodiscard]] bool CreateCommandBuffers();
  [[nodiscard]] bool CreateSyncObjects();

  void ReCreateSwapChain();
  void CleanupSwapChain();

  void RecordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const;

  [[nodiscard]] static vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
  [[nodiscard]] static vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

  // helpers
  [[nodiscard]] static bool              CheckValidationLayerSupport();
  [[nodiscard]] std::vector<const char*> GetRequiredExtensions();
  [[nodiscard]] bool                     IsDeviceSuitable(const vk::PhysicalDevice& device) const;
  [[nodiscard]] bool                     CheckDeviceExtensionSupport(const vk::PhysicalDevice& device) const;
  [[nodiscard]] vk::Extent2D             ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const;


  static void                                         PrintExtensionsSupport();
  [[nodiscard]] static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSensivity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/);

  [[nodiscard]] uint32_t RateDeviceSuitability(const vk::PhysicalDevice& device) const;

private:
  Window<GlfwWindow>&            m_Window;
  vk::Instance                   m_Instance;
  vk::DebugUtilsMessengerEXT     m_DebugMessenger;
  vk::SurfaceKHR                 m_Surface;
  vk::PhysicalDevice             m_PhysicalDevice;
  vk::Device                     m_Device;
  vk::Queue                      m_GraphicsQueue;
  vk::Queue                      m_PresentQueue;
  std::vector<const char*>       m_DeviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  vk::Extent2D                   m_SwapChainExtent;
  vk::SwapchainKHR               m_SwapChain;
  std::vector<vk::Image>         m_SwapChainImages;
  vk::Format                     m_SwapChainImageFormat;
  std::vector<vk::ImageView>     m_SwapChainImageViews;
  vk::RenderPass                 m_RenderPass;
  vk::Pipeline                   m_GraphicsPipeline;
  vk::ShaderModule               m_VertexShaderModule;
  vk::ShaderModule               m_FragmentShaderModule;
  vk::PipelineLayout             m_PipelineLayout;
  std::vector<vk::Framebuffer>   m_SwapChainFramebuffers;
  vk::CommandPool                m_CommandPool;
  std::vector<vk::CommandBuffer> m_CommandBuffers;
  std::vector<vk::Semaphore>     m_ImageAvailableSemaphores;
  std::vector<vk::Semaphore>     m_RenderFinishedSemaphores;
  std::vector<vk::Fence>         m_InFlightFences;
  uint32_t                       m_CurrentFrame{0};
};
} // namespace four
