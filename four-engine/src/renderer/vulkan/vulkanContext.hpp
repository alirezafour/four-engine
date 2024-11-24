// module;
#pragma once

#include "core/core.hpp"

#include "vulkan/vulkan.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include "glm/glm.hpp"

// export module vkContext;
// export
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
  glm::vec2 pos;
  glm::vec3 color;

  static vk::VertexInputBindingDescription GetBindingDescription()
  {
    return {.binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex};
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

struct UniformBufferObject
{
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
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
  [[nodiscard]] bool             CreateDescriptorSetLayout();
  [[nodiscard]] bool             CreateGraphicsPipeline();
  [[nodiscard]] vk::ShaderModule CreateShaderModule(const std::vector<char>& code);
  [[nodiscard]] vk::Format       FindDepthFormat() const;

  [[nodiscard]] bool CreateFramebuffers();
  [[nodiscard]] bool CreateCommandPool();
  [[nodiscard]] bool CreateCommandBuffers();
  [[nodiscard]] bool CreateSyncObjects();

  void ReCreateSwapChain();
  void CleanupSwapChain();

  [[nodiscard]] bool CreateVertexBuffers();
  [[nodiscard]] bool CreateIndexBuffers();
  [[nodiscard]] bool CreateUniformBuffers();

  void               CreateBuffer(vk::DeviceSize          size,
                                  vk::BufferUsageFlags    usage,
                                  vk::MemoryPropertyFlags properties,
                                  vk::Buffer&             buffer,
                                  vk::DeviceMemory&       bufferMemory);
  [[nodiscard]] bool CreateDescriptorPool();
  [[nodiscard]] bool CreateDescriptorSets();
  [[nodiscard]] bool CreateTextureImage();

  /**
   * Creates an image and allocates memory for it.
   * exeption on failure
   * @param width
   * @param height
   * @param format
   * @param tiling
   * @param usage
   * @param properties
   * @return std::tuple<vk::Image, vk::DeviceMemory>
   */
  void CreateImage(uint32_t                width,
                   uint32_t                height,
                   vk::Format              format,
                   vk::ImageTiling         tiling,
                   vk::ImageUsageFlags     usage,
                   vk::MemoryPropertyFlags properties,
                   vk::Image&              textureImage,
                   vk::DeviceMemory&       textureImageMemory) const;

  void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) const;

  void RecordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const;

  [[nodiscard]] vk::CommandBuffer BeginSingleTimeCommands() const;

  void EndSingleTimeCommands(const vk::CommandBuffer& commandBuffer) const;

  void UpdateUniformBuffer(uint32_t currentImage) const;

  void TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;
  void CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) const;

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
  [[nodiscard]] uint32_t FindMemoryType(uint32_t typeFilter, const vk::MemoryPropertyFlags& properties) const;

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
  vk::Buffer                     m_IndexBuffer;
  vk::DeviceMemory               m_IndexBufferMemory;
  const std::vector<Vertex>      vertices{{{-0.5f, -0.5f}, {1.0f, 0.3f, 1.0f}},
                                          {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                          {{0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
                                          {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};
  const std::vector<uint16_t>    indices{0, 1, 2, 2, 3, 0};
  vk::DescriptorSetLayout        m_DescriptorSetLayout;
  std::vector<vk::Buffer>        m_UniformBuffers;
  std::vector<vk::DeviceMemory>  m_UniformBuffersMemory;
  std::vector<void*>             m_UniformBuffersMapped;
  vk::DescriptorPool             m_DescriptorPool;
  std::vector<vk::DescriptorSet> m_DescriptorSets;
  vk::Image                      m_TextureImage;
  vk::DeviceMemory               m_TextureImageMemory;
};
} // namespace four
