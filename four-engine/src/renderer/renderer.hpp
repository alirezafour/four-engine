#pragma once

// test vulkan
#include "renderer/vulkan/vulkDevice.hpp"
#include "renderer/vulkan/vulkSwapChain.hpp"

namespace four
{

class Renderer
{
public:
  /*
   * @brief Default constructor
   * @param vulkDevice
   * @param window
   */
  explicit Renderer(Window<GlfwWindow>& window, VulkDevice& vulkDevice);
  ~Renderer();
  Renderer(const Renderer&)                = delete;
  Renderer& operator=(const Renderer&)     = delete;
  Renderer(Renderer&&) noexcept            = delete;
  Renderer& operator=(Renderer&&) noexcept = delete;

  [[nodiscard]] vk::RenderPass GetSwapChainRenderPass() const
  {
    return m_SwapChain->GetRenderPass();
  }

  [[nodiscard]] bool IsFrameStarted() const
  {
    return m_IsFrameStarted;
  }

  [[nodiscard]] vk::CommandBuffer GetCommandBuffer()
  {
    assert(m_IsFrameStarted && "Cannot get command buffer when frame not started");
    return m_CommandBuffers[m_CurrentFrameIndex];
  }

  [[nodiscard]] int GetFrameIndex() const
  {
    assert(m_IsFrameStarted && "Cannot get frame index when frame not started");
    return m_CurrentFrameIndex;
  }

  vk::CommandBuffer BeginFrame();
  void              EndFrame();
  void              BeginSwapChainRenderPass(vk::CommandBuffer commandBuffer);
  void              EndSwapChainRenderPass(vk::CommandBuffer commandBuffer);

private:
  void CreateCommandBuffers();
  void FreeCommandBuffer();
  void DrawFrame();
  void ReCreateSwapChain();

private:
  VulkDevice&                    m_VulkDevice;
  Window<GlfwWindow>&            m_Window;
  std::unique_ptr<VulkSwapChain> m_SwapChain;
  std::vector<vk::CommandBuffer> m_CommandBuffers;

  uint32_t m_CurrentImageIndex{};
  int      m_CurrentFrameIndex{};
  bool     m_IsFrameStarted{false};
};
} // namespace four
