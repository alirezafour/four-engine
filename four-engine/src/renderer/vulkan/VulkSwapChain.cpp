#include <utility>

#include "four-pch.h"

#include "renderer/vulkan/vulkSwapChain.hpp"
#include "renderer/vulkan/vulkDevice.hpp"

namespace four
{

VulkSwapChain::VulkSwapChain(VulkDevice& deviceRef, WindowExtent extent) :
m_VulkDevice{deviceRef},
m_WindowExtent{extent}
{
  Init();
}

void VulkSwapChain::Init()
{
  CreateSwapChain();
  CreateImageViews();
  CreateRenderPass();
  CreateDepthResources();
  CreateFramebuffers();
  CreateSyncObjects();
}

VulkSwapChain::VulkSwapChain(VulkDevice& deviceRef, WindowExtent extent, std::shared_ptr<VulkSwapChain> prevSwapChain) :
m_VulkDevice{deviceRef},
m_WindowExtent{extent},
m_PrevSwapChain{std::move(prevSwapChain)}
{
  Init();

  // cheanup old swap chain since it's no longer needed
  m_PrevSwapChain.reset();
}

VulkSwapChain::~VulkSwapChain()
{
  auto device = m_VulkDevice.GetDevice();
  for (auto imageView : m_SwapChainImageViews)
  {
    device.destroyImageView(imageView);
  }
  m_SwapChainImageViews.clear();

  if (m_SwapChain != nullptr)
  {
    device.destroySwapchainKHR(m_SwapChain);
    m_SwapChain = nullptr;
  }

  for (int i = 0; i < m_DepthImages.size(); ++i)
  {
    device.destroyImageView(m_DepthImageViews[i]);
    device.destroyImage(m_DepthImages[i]);
    device.freeMemory(m_DepthImageMemorys[i]);
  }

  for (auto framebuffer : m_SwapChainFramebuffers)
  {
    device.destroyFramebuffer(framebuffer);
  }
  device.destroyRenderPass(m_RenderPass);

  // cleanup synchronization objects
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
  {
    device.destroySemaphore(m_RenderFinishedSemaphores[i], nullptr);
    device.destroySemaphore(m_ImageAvailableSemaphores[i], nullptr);
    device.destroyFence(m_InFlightFences[i], nullptr);
  }
}

vk::Result VulkSwapChain::AcquireNextImage(uint32_t* imageIndex)
{
  assert(imageIndex != nullptr && "Cannot acquire next image: image index pointer is null!");

  auto device = m_VulkDevice.GetDevice();
  if (vk::Result result = device.waitForFences(1,
                                               &m_InFlightFences[m_CurrentFrame],
                                               vk::Bool32(true),
                                               std::numeric_limits<uint64_t>::max());
      result != vk::Result::eSuccess)
  {
    throw std::runtime_error("failed to wait for fence!");
  }

  return device.acquireNextImageKHR(m_SwapChain,
                                    std::numeric_limits<uint64_t>::max(),
                                    m_ImageAvailableSemaphores[m_CurrentFrame],
                                    VK_NULL_HANDLE,
                                    imageIndex);
}

vk::Result VulkSwapChain::SubmitCommandBuffers(const vk::CommandBuffer* buffers, uint32_t* imageIndex)
{
  assert(buffers != nullptr && "Cannot submit command buffers: command buffer pointer is null!");
  assert(imageIndex != nullptr && "Cannot submit command buffers: image index pointer is null!");

  auto device = m_VulkDevice.GetDevice();
  if (m_ImagesInFlight[*imageIndex] != nullptr)
  {
    if (auto result = device.waitForFences(1,
                                           &m_ImagesInFlight[*imageIndex],
                                           vk::Bool32(true),
                                           std::numeric_limits<uint64_t>::max());
        result != vk::Result::eSuccess)
    {
      throw std::runtime_error("failed to wait for fence!");
    }
  }
  m_ImagesInFlight[*imageIndex] = m_InFlightFences[m_CurrentFrame];

  vk::SubmitInfo submitInfo = {};
  submitInfo.sType          = vk::StructureType::eSubmitInfo;

  std::array<vk::Semaphore, 1>          waitSemaphores{m_ImageAvailableSemaphores[m_CurrentFrame]};
  std::array<vk::PipelineStageFlags, 1> waitStages{vk::PipelineStageFlagBits::eColorAttachmentOutput};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores    = waitSemaphores.data();
  submitInfo.pWaitDstStageMask  = waitStages.data();

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = buffers;

  std::array<vk::Semaphore, 1> signalSemaphores{m_RenderFinishedSemaphores[m_CurrentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores    = signalSemaphores.data();

  if (auto result = device.resetFences(1, &m_InFlightFences[m_CurrentFrame]); result != vk::Result::eSuccess)
  {
    throw std::runtime_error("failed to reset fence!");
  }

  if (auto result = m_VulkDevice.GetGraphicsQueue().submit(1, &submitInfo, m_InFlightFences[m_CurrentFrame]);
      result != vk::Result::eSuccess)
  {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  vk::PresentInfoKHR presentInfo = {};
  presentInfo.sType              = vk::StructureType::ePresentInfoKHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores    = signalSemaphores.data();

  std::array<vk::SwapchainKHR, 1> swapChains{m_SwapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains    = swapChains.data();
  presentInfo.pImageIndices  = imageIndex;

  auto result = m_VulkDevice.GetPresentQueue().presentKHR(&presentInfo);

  m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

  return result;
}

void VulkSwapChain::CreateSwapChain()
{
  SwapChainSupportDetails swapChainSupport = m_VulkDevice.GetSwapChainSupport();

  vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
  vk::PresentModeKHR   presentMode   = ChooseSwapPresentMode(swapChainSupport.presentModes);
  vk::Extent2D         extent        = ChooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
  {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR createInfo{vk::SwapchainCreateFlagsKHR(),
                                        m_VulkDevice.GetSurface(),
                                        imageCount,
                                        surfaceFormat.format,
                                        surfaceFormat.colorSpace,
                                        extent,
                                        1,
                                        vk::ImageUsageFlagBits::eColorAttachment};

  QueueFamilyIndices indices = m_VulkDevice.FindPhysicalQueueFamilies();
  std::array         queueFamilyIndices{indices.graphicsFamily, indices.presentFamily};

  if (indices.graphicsFamily != indices.presentFamily)
  {
    createInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
  }
  else
  {
    createInfo.imageSharingMode      = vk::SharingMode::eExclusive;
    createInfo.queueFamilyIndexCount = 0;       // Optional
    createInfo.pQueueFamilyIndices   = nullptr; // Optional
  }

  createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

  createInfo.presentMode = presentMode;
  createInfo.clipped     = vk::Bool32(true);

  createInfo.oldSwapchain = m_PrevSwapChain ? m_PrevSwapChain->GetSwapChainHandle() : nullptr;

  auto device = m_VulkDevice.GetDevice();

  m_SwapChain = device.createSwapchainKHR(createInfo);

  // we only specified a minimum number of images in the swap chain, so the implementation is
  // allowed to create a swap chain with more. That's why we'll first query the final number of
  // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
  // retrieve the handles.
  if (auto result = device.getSwapchainImagesKHR(m_SwapChain, &imageCount, nullptr); result != vk::Result::eSuccess)
  {
    throw std::runtime_error("failed to get number of swap chain images!");
  }
  m_SwapChainImages.resize(imageCount);

  if (auto result = device.getSwapchainImagesKHR(m_SwapChain, &imageCount, m_SwapChainImages.data());
      result != vk::Result::eSuccess)
  {
    throw std::runtime_error("failed to get swap chain images!");
  }

  m_SwapChainImageFormat = surfaceFormat.format;
  m_SwapChainExtent      = extent;
}

void VulkSwapChain::CreateImageViews()
{
  m_SwapChainImageViews.resize(m_SwapChainImages.size());
  for (size_t i = 0; i < m_SwapChainImages.size(); ++i)
  {
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.sType                           = vk::StructureType::eImageViewCreateInfo;
    viewInfo.image                           = m_SwapChainImages[i];
    viewInfo.viewType                        = vk::ImageViewType::e2D;
    viewInfo.format                          = m_SwapChainImageFormat;
    viewInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;

    if (auto result = m_VulkDevice.GetDevice().createImageView(&viewInfo, nullptr, &m_SwapChainImageViews[i]);
        result != vk::Result::eSuccess)
    {
      throw std::runtime_error("failed to create texture image view!");
    }
  }
}

void VulkSwapChain::CreateRenderPass()
{
  vk::AttachmentDescription depthAttachment{};
  depthAttachment.format         = FindDepthFormat();
  depthAttachment.samples        = vk::SampleCountFlagBits::e1;
  depthAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
  depthAttachment.storeOp        = vk::AttachmentStoreOp::eDontCare;
  depthAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
  depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  depthAttachment.initialLayout  = vk::ImageLayout::eUndefined;
  depthAttachment.finalLayout    = vk::ImageLayout::eDepthStencilAttachmentOptimal;

  vk::AttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout     = vk::ImageLayout::eDepthStencilAttachmentOptimal;

  vk::AttachmentDescription colorAttachment = {};
  colorAttachment.format                    = GetSwapChainImageFormat();
  colorAttachment.samples                   = vk::SampleCountFlagBits::e1;
  colorAttachment.loadOp                    = vk::AttachmentLoadOp::eClear;
  colorAttachment.storeOp                   = vk::AttachmentStoreOp::eStore;
  colorAttachment.stencilStoreOp            = vk::AttachmentStoreOp::eDontCare;
  colorAttachment.stencilLoadOp             = vk::AttachmentLoadOp::eDontCare;
  colorAttachment.initialLayout             = vk::ImageLayout::eUndefined;
  colorAttachment.finalLayout               = vk::ImageLayout::ePresentSrcKHR;

  vk::AttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment              = 0;
  colorAttachmentRef.layout                  = vk::ImageLayout::eColorAttachmentOptimal;

  vk::SubpassDescription subpass  = {};
  subpass.pipelineBindPoint       = vk::PipelineBindPoint::eGraphics;
  subpass.colorAttachmentCount    = 1;
  subpass.pColorAttachments       = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  vk::SubpassDependency dependency = {};
  dependency.srcSubpass            = vk::SubpassExternal;
  dependency.srcAccessMask         = vk::AccessFlagBits::eNone;
  dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
  dependency.dstSubpass = 0;
  dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
  dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

  std::array<vk::AttachmentDescription, 2> attachments    = {colorAttachment, depthAttachment};
  vk::RenderPassCreateInfo                 renderPassInfo = {};
  renderPassInfo.sType                                    = vk::StructureType::eRenderPassCreateInfo;
  renderPassInfo.attachmentCount                          = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments                             = attachments.data();
  renderPassInfo.subpassCount                             = 1;
  renderPassInfo.pSubpasses                               = &subpass;
  renderPassInfo.dependencyCount                          = 1;
  renderPassInfo.pDependencies                            = &dependency;

  if (auto result = m_VulkDevice.GetDevice().createRenderPass(&renderPassInfo, nullptr, &m_RenderPass);
      result != vk::Result::eSuccess)
  {
    throw std::runtime_error("failed to create render pass!");
  }
}

void VulkSwapChain::CreateFramebuffers()
{
  m_SwapChainFramebuffers.resize(ImageCount());
  for (size_t i = 0; i < ImageCount(); ++i)
  {
    std::array<vk::ImageView, 2> attachments = {m_SwapChainImageViews[i], m_DepthImageViews[i]};

    vk::Extent2D              swapChainExtent = GetSwapChainExtent();
    vk::FramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType                     = vk::StructureType::eFramebufferCreateInfo;
    framebufferInfo.renderPass                = m_RenderPass;
    framebufferInfo.attachmentCount           = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments              = attachments.data();
    framebufferInfo.width                     = swapChainExtent.width;
    framebufferInfo.height                    = swapChainExtent.height;
    framebufferInfo.layers                    = 1;

    if (auto result = m_VulkDevice.GetDevice().createFramebuffer(&framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]);
        result != vk::Result::eSuccess)
    {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void VulkSwapChain::CreateDepthResources()
{
  vk::Format depthFormat       = FindDepthFormat();
  m_DepthFormat                = depthFormat;
  vk::Extent2D swapChainExtent = GetSwapChainExtent();

  m_DepthImages.resize(ImageCount());
  m_DepthImageMemorys.resize(ImageCount());
  m_DepthImageViews.resize(ImageCount());

  for (int i = 0; i < m_DepthImages.size(); ++i)
  {
    vk::ImageCreateInfo imageInfo{};
    imageInfo.sType         = vk::StructureType::eImageCreateInfo;
    imageInfo.imageType     = vk::ImageType::e2D;
    imageInfo.extent.width  = swapChainExtent.width;
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;
    imageInfo.format        = depthFormat;
    imageInfo.tiling        = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage         = vk::ImageUsageFlagBits::eDepthStencilAttachment;
    imageInfo.samples       = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode   = vk::SharingMode::eExclusive;

    m_VulkDevice
      .CreateImageWithInfo(imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, m_DepthImages[i], m_DepthImageMemorys[i]);

    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.sType                           = vk::StructureType::eImageViewCreateInfo;
    viewInfo.image                           = m_DepthImages[i];
    viewInfo.viewType                        = vk::ImageViewType::e2D;
    viewInfo.format                          = depthFormat;
    viewInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eDepth;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;

    if (auto result = m_VulkDevice.GetDevice().createImageView(&viewInfo, nullptr, &m_DepthImageViews[i]);
        result != vk::Result::eSuccess)
    {
      throw std::runtime_error("failed to create texture image view!");
    }
  }
}

void VulkSwapChain::CreateSyncObjects()
{
  m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  m_ImagesInFlight.resize(ImageCount(), VK_NULL_HANDLE);

  vk::SemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType                   = vk::StructureType::eSemaphoreCreateInfo;

  vk::FenceCreateInfo fenceInfo = {};
  fenceInfo.sType               = vk::StructureType::eFenceCreateInfo;
  fenceInfo.flags               = vk::FenceCreateFlagBits::eSignaled;

  auto device = m_VulkDevice.GetDevice();
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
  {
    if (auto result = device.createSemaphore(&semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]);
        result != vk::Result::eSuccess)
    {
      throw std::runtime_error("failed to create semaphore!");
    }
    if (auto result = device.createSemaphore(&semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]);
        result != vk::Result::eSuccess)
    {
      throw std::runtime_error("failed to create semaphore!");
    }
    if (auto result = device.createFence(&fenceInfo, nullptr, &m_InFlightFences[i]); result != vk::Result::eSuccess)
    {
      throw std::runtime_error("failed to create fence!");
    }
  }
}

vk::SurfaceFormatKHR VulkSwapChain::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
  for (const auto& availableFormat : availableFormats)
  {
    if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
        availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
    {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

vk::PresentModeKHR VulkSwapChain::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
  for (const auto& availablePresentMode : availablePresentModes)
  {
    if (availablePresentMode == vk::PresentModeKHR::eMailbox)
    {
      LOG_CORE_INFO("Present mode: Mailbox");
      return availablePresentMode;
    }
  }

  // for (const auto &availablePresentMode : availablePresentModes) {
  //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
  //     LOG_CORE_INFO ("Present mode: Immediate");
  //     return availablePresentMode;
  //   }
  // }

  LOG_CORE_INFO("Present mode: V-Sync\n");
  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkSwapChain::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
{
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
  {
    return capabilities.currentExtent;
  }
  else
  {
    vk::Extent2D actualExtent = {m_WindowExtent.width, m_WindowExtent.height};
    actualExtent.width        = std::max(capabilities.minImageExtent.width,
                                  std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height       = std::max(capabilities.minImageExtent.height,
                                   std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

vk::Format VulkSwapChain::FindDepthFormat()
{
  return m_VulkDevice.FindSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                                          vk::ImageTiling::eOptimal,
                                          vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

} //namespace four
