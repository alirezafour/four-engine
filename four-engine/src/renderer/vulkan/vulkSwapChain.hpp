#pragma once

#include "vulkan/vulkan_core.h"
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


  [[nodiscard]] VkFramebuffer GetFrameBuffer(int index) const
  {
    return m_SwapChainFramebuffers[index];
  }
  [[nodiscard]] VkRenderPass GetRenderPass() const
  {
    return m_RenderPass;
  }
  [[nodiscard]] VkImageView GetImageView(int index) const
  {
    return m_SwapChainImageViews[index];
  }
  [[nodiscard]] size_t ImageCount() const
  {
    return m_SwapChainImages.size();
  }
  [[nodiscard]] VkFormat GetSwapChainImageFormat() const
  {
    return m_SwapChainImageFormat;
  }
  [[nodiscard]] VkExtent2D GetSwapChainExtent() const
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
  [[nodiscard]] VkFormat FindDepthFormat();

  [[nodiscard]] VkResult AcquireNextImage(uint32_t* imageIndex);
  [[nodiscard]] VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

  [[nodiscard]] VkSwapchainKHR GetSwapChainHandle() const
  {
    return m_SwapChain;
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
  [[nodiscard]] VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
  [[nodiscard]] VkPresentModeKHR   ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
  [[nodiscard]] VkExtent2D         ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

private:
  VkFormat   m_SwapChainImageFormat;
  VkExtent2D m_SwapChainExtent;

  std::vector<VkFramebuffer>     m_SwapChainFramebuffers;
  VkRenderPass                   m_RenderPass;
  std::shared_ptr<VulkSwapChain> m_PrevSwapChain;

  std::vector<VkImage>        m_DepthImages;
  std::vector<VkDeviceMemory> m_DepthImageMemorys;
  std::vector<VkImageView>    m_DepthImageViews;
  std::vector<VkImage>        m_SwapChainImages;
  std::vector<VkImageView>    m_SwapChainImageViews;

  VulkDevice&  m_VulkDevice;
  WindowExtent m_WindowExtent;

  VkSwapchainKHR m_SwapChain;

  std::vector<VkSemaphore> m_ImageAvailableSemaphores;
  std::vector<VkSemaphore> m_RenderFinishedSemaphores;
  std::vector<VkFence>     m_InFlightFences;
  std::vector<VkFence>     m_ImagesInFlight;
  size_t                   m_CurrentFrame = 0;
};

} // namespace four
