#include "four-pch.h"

#include "renderer/renderer.hpp"
#include "renderer/vulkan/vulkSwapChain.hpp"
#include "renderer/vulkan/vulkDevice.hpp"
#include "window/glfw/glfwWindow.hpp"

namespace four
{

//====================================================================================
Renderer::Renderer(Window<GlfwWindow>& window, VulkDevice& vulkDevice) : m_Window{window}, m_VulkDevice{vulkDevice}
{
  ReCreateSwapChain();
  CreateCommandBuffers();
}

//====================================================================================
Renderer::~Renderer()
{
  FreeCommandBuffer();
}

//====================================================================================
void Renderer::ReCreateSwapChain()
{
  auto extent = m_Window.GetExtent();
  while (extent.width == 0 || extent.height == 0)
  {
    extent = m_Window.GetExtent();
    m_Window.WaitEvents();
  }

  vkDeviceWaitIdle(m_VulkDevice.GetDevice());

  // if already have a valid swapchain, pass it to VulkSwapChain
  if (m_SwapChain)
  {
    std::shared_ptr<VulkSwapChain> oldSwapChain = std::move(m_SwapChain);
    m_SwapChain                                 = std::make_unique<VulkSwapChain>(m_VulkDevice, extent, oldSwapChain);

    if (!oldSwapChain->CompareSwapFormats(*m_SwapChain))
    {
      // TODO: later handle it with call back to notify the engine that is not compatible
      throw std::runtime_error("Swap chain image or depth format has changed!");
    }
  }
  else
  {
    m_SwapChain = std::make_unique<VulkSwapChain>(m_VulkDevice, extent);
  }

  // TODO: we come back here
}


//====================================================================================
void Renderer::FreeCommandBuffer()
{
  vkFreeCommandBuffers(m_VulkDevice.GetDevice(),
                       m_VulkDevice.GetCommandPool(),
                       static_cast<uint32_t>(m_CommandBuffers.size()),
                       m_CommandBuffers.data());

  m_CommandBuffers.clear();
}

//====================================================================================
void Renderer::CreateCommandBuffers()
{
  m_CommandBuffers.resize(VulkSwapChain::MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool        = m_VulkDevice.GetCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

  if (vkAllocateCommandBuffers(m_VulkDevice.GetDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
  {
    LOG_CORE_ERROR("failed to allocate command buffers!");
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

//====================================================================================
VkCommandBuffer Renderer::BeginFrame()
{
  assert(!m_IsFrameStarted && "Can't call BeginFrame while frame is in progress");
  uint32_t imageIndex = 0;
  VkResult result     = m_SwapChain->AcquireNextImage(&imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    ReCreateSwapChain();
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
  {
    throw std::runtime_error("failed to acquire swap chain image");
  }

  m_IsFrameStarted = true;

  auto*                    commandBuffer = GetCommandBuffer();
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  return commandBuffer;
}

//====================================================================================
void Renderer::EndFrame()
{
  assert(m_IsFrameStarted && "Can't call EndFrame while frame is not in progress");
  auto* commandBuffer = GetCommandBuffer();
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to record command buffer!");
  }

  auto result = m_SwapChain->SubmitCommandBuffers(&commandBuffer, &m_CurrentImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window.WasWindowResized())
  {
    m_Window.ResetWindowResized();
    ReCreateSwapChain();
  }
  else if (result != VK_SUCCESS)
  {
    throw std::runtime_error("failed to present swap chain image");
  }

  m_IsFrameStarted    = false;
  m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % VulkSwapChain::MAX_FRAMES_IN_FLIGHT;
}
void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
  assert(m_IsFrameStarted && "Can't call BeginSwapChainRenderPass if frame is not in progress");
  assert(commandBuffer == GetCommandBuffer() && "Can't begin render pass on command buffer from a different frame");


  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass  = m_SwapChain->GetRenderPass();
  renderPassInfo.framebuffer = m_SwapChain->GetFrameBuffer(static_cast<int>(m_CurrentImageIndex));

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color           = {0.01F, 0.01F, 0.01F, 1.0F};
  clearValues[1].depthStencil    = {1.0F, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues    = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x        = 0.0F;
  viewport.y        = 0.0F;
  viewport.width    = static_cast<float>(m_SwapChain->GetSwapChainExtent().width);
  viewport.height   = static_cast<float>(m_SwapChain->GetSwapChainExtent().height);
  viewport.minDepth = 0.0F;
  viewport.maxDepth = 1.0F;
  VkRect2D scissor{{0, 0}, m_SwapChain->GetSwapChainExtent()};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}
void Renderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
  assert(m_IsFrameStarted && "Can't call EndSwapChainRenderPass if frame is not in progress");
  assert(commandBuffer == GetCommandBuffer() &&
         "Can't end render pass on command buffer from a different "
         "frame");
  vkCmdEndRenderPass(commandBuffer);
}

} // namespace four
