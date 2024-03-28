#pragma once

#include "vulkan/vulkan.hpp"
#include "window/window.hpp"

namespace four
{
class VulkDevice;

class VulkSwapChain
{
public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  explicit VulkSwapChain(VulkDevice& device, WindowExtent windowExtent);
  explicit VulkSwapChain(VulkDevice& device, WindowExtent windowExtent, std::shared_ptr<VulkSwapChain> prevSwapChain);
  ~VulkSwapChain();

  VulkSwapChain(const VulkSwapChain&)            = delete;
  VulkSwapChain& operator=(const VulkSwapChain&) = delete;
  VulkSwapChain(VulkSwapChain&&)                 = delete;
  VulkSwapChain& operator=(VulkSwapChain&&)      = delete;


  [[nodiscard]] vk::Framebuffer GetFrameBuffer(int index) const
  {
    return m_SwapChainFramebuffers[index];
  }
  [[nodiscard]] vk::RenderPass GetRenderPass() const
  {
    return m_RenderPass;
  }
  [[nodiscard]] vk::ImageView GetImageView(int index) const
  {
    return m_SwapChainImageViews[index];
  }
  [[nodiscard]] size_t ImageCount() const
  {
    return m_SwapChainImages.size();
  }
  [[nodiscard]] vk::Format GetSwapChainImageFormat() const
  {
    return m_SwapChainImageFormat;
  }
  [[nodiscard]] vk::Extent2D GetSwapChainExtent() const
  {
    return m_SwapChainExtent;
  }
  [[nodiscard]] uint32_t GetWidth() const
  {
    return m_SwapChainExtent.width;
  }
  [[nodiscard]] uint32_t GetHeight() const
  {
    return m_SwapChainExtent.height;
  }

  [[nodiscard]] float ExtentAspectRatio() const
  {
    return static_cast<float>(m_SwapChainExtent.width) / static_cast<float>(m_SwapChainExtent.height);
  }
  [[nodiscard]] vk::Format FindDepthFormat();

  [[nodiscard]] vk::Result AcquireNextImage(uint32_t* imageIndex);
  [[nodiscard]] vk::Result SubmitCommandBuffers(const vk::CommandBuffer* buffers, uint32_t* imageIndex);

  [[nodiscard]] vk::SwapchainKHR GetSwapChainHandle() const
  {
    return m_SwapChain;
  }

  [[nodiscard]] bool CompareSwapFormats(const VulkSwapChain& swapChain) const
  {
    return (m_SwapChainImageFormat == swapChain.m_SwapChainImageFormat) && (m_DepthFormat == swapChain.m_DepthFormat);
  }

private:
  void Init();
  void CreateSwapChain();
  void CreateImageViews();
  void CreateDepthResources();
  void CreateRenderPass();
  void CreateFramebuffers();
  void CreateSyncObjects();

  // Helper functions
  [[nodiscard]] vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
  [[nodiscard]] vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
  [[nodiscard]] vk::Extent2D       ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const;

private:
  vk::Format   m_SwapChainImageFormat;
  vk::Format   m_DepthFormat;
  vk::Extent2D m_SwapChainExtent;

  std::vector<vk::Framebuffer>   m_SwapChainFramebuffers;
  vk::RenderPass                 m_RenderPass;
  std::shared_ptr<VulkSwapChain> m_PrevSwapChain;

  std::vector<vk::Image>        m_DepthImages;
  std::vector<vk::DeviceMemory> m_DepthImageMemorys;
  std::vector<vk::ImageView>    m_DepthImageViews;
  std::vector<vk::Image>        m_SwapChainImages;
  std::vector<vk::ImageView>    m_SwapChainImageViews;

  VulkDevice&  m_VulkDevice;
  WindowExtent m_WindowExtent;

  vk::SwapchainKHR m_SwapChain;

  std::vector<vk::Semaphore> m_ImageAvailableSemaphores;
  std::vector<vk::Semaphore> m_RenderFinishedSemaphores;
  std::vector<vk::Fence>     m_InFlightFences;
  std::vector<vk::Fence>     m_ImagesInFlight;
  size_t                     m_CurrentFrame = 0;
};

} // namespace four
