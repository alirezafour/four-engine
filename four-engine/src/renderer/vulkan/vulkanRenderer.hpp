// module;
#pragma once

#include "core/core.hpp"

#include "renderer/renderer.hpp"
#include "vulkan/vulkan.hpp"

#include "window/glfw/glfwWindow.hpp"

#include "camera/camera.hpp"
#include <vulkan/vulkan_handles.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// export module vkContext;
// export
namespace four
{

#ifdef NDEBUG
constexpr bool EnableValidationLayers = true;
#else
constexpr bool EnableValidationLayers = false;
#endif

constexpr std::array<const char*, 1> ValidationLayers = {"VK_LAYER_KHRONOS_validation"};

struct Vertex
{
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 texCoord;

  static vk::VertexInputBindingDescription GetBindingDescription()
  {
    return {.binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex};
  }

  static std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescriptions()
  {
    using VIAD = vk::VertexInputAttributeDescription;
    return {VIAD{.location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, pos)},
            VIAD{.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, color)},
            VIAD{.location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, texCoord)}};
  }
};

struct UniformBufferObject
{
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};


constexpr int MAX_FRAMES_IN_FLIGHT = 2;

class FOUR_ENGINE_API VulkanRenderer final : public Renderer<VulkanRenderer>
{
  friend class Renderer<VulkanRenderer>;

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

  struct DeletionQueue
  {
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()>&& function)
    {
      deletors.emplace_back(std::move(function));
    }

    void flush()
    {
      for (auto it = deletors.rbegin(); it != deletors.rend(); ++it)
      {
        auto& function = *it;
        function();
        deletors.clear();
      }
    }
  };
  struct FrameData
  {
    vk::Fence     inFlightFence;
    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;

    vk::CommandPool   commandPool;
    vk::CommandBuffer commandBuffer;

    DeletionQueue deletionQueue;
  };

  explicit VulkanRenderer(WindowType& window);
  ~VulkanRenderer() final;

  VulkanRenderer(const VulkanRenderer&)            = delete;
  VulkanRenderer(VulkanRenderer&&)                 = delete;
  VulkanRenderer& operator=(const VulkanRenderer&) = delete;
  VulkanRenderer& operator=(VulkanRenderer&&)      = delete;

protected:
  void DrawFrame();
  void DrawImGui(vk::CommandBuffer cmd, vk::ImageView targetImageView) const;
  void StopRenderImpl();

  [[nodiscard]] static SwapChainSupportDetails QuerySwapChainSupport(const vk::PhysicalDevice& device,
                                                                     const vk::SurfaceKHR&     surface);
  [[nodiscard]] static QueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);
  [[nodiscard]] const vk::Instance& GetVulkanInstance() const
  {
    return m_Instance;
  }
  [[nodiscard]] const vk::PhysicalDevice& GetPhysicalDevice() const
  {
    return m_PhysicalDevice;
  }
  [[nodiscard]] const vk::SurfaceKHR& GetSurface() const
  {
    return m_Surface;
  }
  [[nodiscard]] const std::vector<const char*>& GetDeviceExtensions() const
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

  void ImmediateSubmit(std::function<void(vk::CommandBuffer commandBuffer)>&& function);

private:
  [[nodiscard]] bool InitVulkan();
  void               ShutdownVulkan();

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
  [[nodiscard]] bool             HasStencilComponent(vk::Format format) const;


  [[nodiscard]] bool CreateFramebuffers();
  [[nodiscard]] bool CreateCommandPool();
  [[nodiscard]] bool CreateCommandBuffers();
  [[nodiscard]] bool CreateSyncObjects();

  void ReCreateSwapChain();
  void CleanupSwapChain();

  [[nodiscard]] bool CreateVertexBuffers();
  [[nodiscard]] bool CreateIndexBuffers();
  [[nodiscard]] bool CreateUniformBuffers();

  /**
   * Creates a buffer and allocates memory for it.
   * exeption on failure
   * @param size
   * @param usage
   * @param properties
   * @param buffer ref to buffer that will be created
   * @param bufferMemory ref to memory that will be allocated
   */
  void CreateBuffer(vk::DeviceSize          size,
                    vk::BufferUsageFlags    usage,
                    vk::MemoryPropertyFlags properties,
                    vk::Buffer&             buffer,
                    vk::DeviceMemory&       bufferMemory);

  [[nodiscard]] bool CreateDescriptorPool();
  [[nodiscard]] bool CreateDescriptorSets();
  [[nodiscard]] bool CreateDepthResources();
  [[nodiscard]] bool CreateTextureImage();
  [[nodiscard]] bool CreateTextureImageView();
  [[nodiscard]] bool CreateTextureSampler();


  /**
   * Creates an image and allocates memory for it.
   * exeption on failure
   * @param width
   * @param height
   * @param format
   * @param tiling
   * @param usage
   * @param properties
   * @param textureImage ref to image that will be created
   * @param textureImageMemory ref to memory that will be allocated
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

  void UpdateUniformBuffer(uint32_t currentImage);

  void TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;
  void CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) const;
  void CopyImageToImage(vk::CommandBuffer cmd,
                        vk::Image         srcImage,
                        vk::Image         dstImage,
                        vk::Extent2D      drawExtent,
                        vk::Extent2D      swapExtent) const;

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

  [[nodiscard]] vk::ImageView CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspect) const;

  [[nodiscard]] FrameData GetCurrentFrameData() const
  {
    return m_FrameData[m_CurrentFrame];
  }

  [[nodiscard]] bool InitImGui();

private:
  WindowType&                    m_Window;
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
  std::vector<FrameData>         m_FrameData;
  DeletionQueue                  m_MainDeletionQueue;
  // Immediate commands
  vk::Fence                 m_ImmediateFence;
  vk::CommandPool           m_ImmediateCommandPool;
  vk::CommandBuffer         m_ImmediateCommandBuffer;
  uint32_t                  m_CurrentFrame{0};
  vk::Buffer                m_VertexBuffer;
  vk::DeviceMemory          m_VertexBufferMemory;
  vk::Buffer                m_IndexBuffer;
  vk::DeviceMemory          m_IndexBufferMemory;
  const std::vector<Vertex> vertices{
    {.pos = {-0.5F, -0.5F, 0.0F}, .color = {1.0F, 0.0F, 0.0F}, .texCoord = {1.0F, 0.0F}},
    {.pos = {0.5F, -0.5F, 0.0F}, .color = {0.0F, 1.0F, 0.0F}, .texCoord = {0.0F, 0.0F}},
    {.pos = {0.5F, 0.5F, 0.0F}, .color = {0.0F, 0.0F, 1.0F}, .texCoord = {0.0F, 1.0F}},
    {.pos = {-0.5F, 0.5F, 0.0F}, .color = {1.0F, 1.0F, 1.0F}, .texCoord = {1.0F, 1.0F}},
    //
    {.pos = {-0.5F, -0.5F, -0.5F}, .color = {1.0F, 0.0F, 0.0F}, .texCoord = {1.0F, 0.0F}},
    {.pos = {0.5F, -0.5F, -0.5F}, .color = {0.0F, 1.0F, 0.0F}, .texCoord = {0.0F, 0.0F}},
    {.pos = {0.5F, 0.5F, -0.5F}, .color = {0.0F, 0.0F, 1.0F}, .texCoord = {0.0F, 1.0F}},
    {.pos = {-0.5F, 0.5F, -0.5F}, .color = {1.0F, 1.0F, 1.0F}, .texCoord = {1.0F, 1.0F}},
  };
  const std::vector<uint16_t>    indices{0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};
  vk::DescriptorSetLayout        m_DescriptorSetLayout;
  std::vector<vk::Buffer>        m_UniformBuffers;
  std::vector<vk::DeviceMemory>  m_UniformBuffersMemory;
  std::vector<void*>             m_UniformBuffersMapped;
  vk::DescriptorPool             m_DescriptorPool;
  std::vector<vk::DescriptorSet> m_DescriptorSets;
  vk::Image                      m_TextureImage;
  vk::DeviceMemory               m_TextureImageMemory;
  vk::ImageView                  m_TextureImageView;
  vk::Sampler                    m_TextureSampler;
  vk::Image                      m_DepthImage;
  vk::DeviceMemory               m_DepthImageMemory;
  vk::ImageView                  m_DepthImageView;

  UniformBufferObject sceneUBO;
  Camera              m_MainCamera;
};
using RendererType = VulkanRenderer;
} // namespace four
