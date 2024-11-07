#pragma once

#include "core/core.hpp"

#include "vulkan/vulkan.hpp"

#include "glm/glm.hpp"

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

struct Vertex
{
  glm::vec2                                pos;
  glm::vec3                                color;
  static vk::VertexInputBindingDescription GetBindingDescription()
  {
    return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
  }

  static std::array<vk::VertexInputAttributeDescription, 2> GetAttributeDescriptions()
  {
    std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};
    attributeDescriptions[0].binding  = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format   = vk::Format::eR32G32Sfloat;
    attributeDescriptions[0].offset   = offsetof(Vertex, pos);
    attributeDescriptions[1].binding  = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format   = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[1].offset   = offsetof(Vertex, color);
    return attributeDescriptions;
  }
};


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
  bool CreateVertexBuffers();

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
  uint32_t               FindMemoryType(uint32_t typeFilter, const vk::MemoryPropertyFlags& properties) const;

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
  vk::Format                     m_SwapChainImageFormat{};
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
  vk::Buffer                     m_VertexBuffer;
  vk::DeviceMemory               m_VertexBufferMemory;
  const std::vector<Vertex>      vertices{{{0.0f, -0.5f}, {1.0f, 0.3f, 1.0f}},
                                          {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                                          {{-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}}};
};
} // namespace four
