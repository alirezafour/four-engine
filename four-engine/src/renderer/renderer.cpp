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

  m_VulkDevice.GetDevice().waitIdle();

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
  m_VulkDevice.GetDevice().freeCommandBuffers(m_VulkDevice.GetCommandPool(), m_CommandBuffers);
  m_CommandBuffers.clear();
}

//====================================================================================
void Renderer::CreateCommandBuffers()
{
  m_CommandBuffers.resize(VulkSwapChain::MAX_FRAMES_IN_FLIGHT);

  vk::CommandBufferAllocateInfo allocInfo{m_VulkDevice.GetCommandPool(),
                                          vk::CommandBufferLevel::ePrimary,
                                          static_cast<uint32_t>(m_CommandBuffers.size())};
  m_CommandBuffers = m_VulkDevice.GetDevice().allocateCommandBuffers(allocInfo);
}

//====================================================================================
vk::CommandBuffer Renderer::BeginFrame()
{
  assert(!m_IsFrameStarted && "Can't call BeginFrame while frame is in progress");
  uint32_t   imageIndex = 0;
  vk::Result result     = m_SwapChain->AcquireNextImage(&imageIndex);

  if (result == vk::Result::eErrorOutOfDateKHR)
  {
    ReCreateSwapChain();
    return nullptr;
  }

  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
  {
    throw std::runtime_error("failed to acquire swap chain image");
  }

  m_IsFrameStarted = true;

  auto                       commandBuffer = GetCommandBuffer();
  vk::CommandBufferBeginInfo beginInfo{};
  beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;

  commandBuffer.begin(beginInfo);
  return commandBuffer;
}

//====================================================================================
void Renderer::EndFrame()
{
  assert(m_IsFrameStarted && "Can't call EndFrame while frame is not in progress");

  auto commandBuffer = GetCommandBuffer();
  commandBuffer.end();

  auto result = m_SwapChain->SubmitCommandBuffers(&commandBuffer, &m_CurrentImageIndex);

  if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_Window.WasWindowResized())
  {
    m_Window.ResetWindowResized();
    ReCreateSwapChain();
  }
  else if (result != vk::Result::eSuccess)
  {
    throw std::runtime_error("failed to present swap chain image");
  }

  m_IsFrameStarted    = false;
  m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % VulkSwapChain::MAX_FRAMES_IN_FLIGHT;
}
void Renderer::BeginSwapChainRenderPass(vk::CommandBuffer commandBuffer)
{
  assert(m_IsFrameStarted && "Can't call BeginSwapChainRenderPass if frame is not in progress");
  assert(commandBuffer == GetCommandBuffer() && "Can't begin render pass on command buffer from a different frame");


  vk::RenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType       = vk::StructureType::eRenderPassBeginInfo;
  renderPassInfo.renderPass  = m_SwapChain->GetRenderPass();
  renderPassInfo.framebuffer = m_SwapChain->GetFrameBuffer(static_cast<int>(m_CurrentImageIndex));

  renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
  renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();

  std::array<vk::ClearValue, 2> clearValues{};
  clearValues[0].color           = {0.01F, 0.01F, 0.01F, 1.0F};
  clearValues[1].depthStencil    = vk::ClearDepthStencilValue{1.0F, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues    = clearValues.data();

  commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

  vk::Viewport viewport{};
  viewport.x        = 0.0F;
  viewport.y        = 0.0F;
  viewport.width    = static_cast<float>(m_SwapChain->GetSwapChainExtent().width);
  viewport.height   = static_cast<float>(m_SwapChain->GetSwapChainExtent().height);
  viewport.minDepth = 0.0F;
  viewport.maxDepth = 1.0F;
  vk::Rect2D scissor{{0, 0}, m_SwapChain->GetSwapChainExtent()};

  commandBuffer.setViewport(0, 1, &viewport);
  commandBuffer.setScissor(0, 1, &scissor);
}
void Renderer::EndSwapChainRenderPass(vk::CommandBuffer commandBuffer)
{
  assert(m_IsFrameStarted && "Can't call EndSwapChainRenderPass if frame is not in progress");
  assert(commandBuffer == GetCommandBuffer() &&
         "Can't end render pass on command buffer from a different "
         "frame");

  commandBuffer.endRenderPass();
}

} // namespace four
