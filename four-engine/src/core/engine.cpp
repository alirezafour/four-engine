#include "four-pch.h"

#include "core/engine.hpp"

namespace four
{
std::unique_ptr<Engine> Engine::sm_Instance = nullptr;

Engine::Engine(std::string_view title, uint32_t width, uint32_t height) : m_Window(nullptr)
{
  if (!InitLog() || !InitWindow(title, width, height))
  {
    LOG_CORE_ERROR("Failed Initializing Engine.");
    return;
  }

  m_ImGuiLayer.PushLayer(std::make_unique<ImGuiLayer>());
  m_ImGuiLayer.PushLayer(std::make_unique<ImGuiLayer>());
  m_VulkDevice = std::make_unique<VulkDevice>(m_Window.get());
  m_VulkDevice->InitVulkan();

  LoadModels();
  CreatePipeLineLayout();
  ReCreateSwapChain();
  CreateCommandBuffers();
}

Engine::~Engine()
{
  vkDestroyPipelineLayout(m_VulkDevice->GetDevice(), m_PipelineLayout, nullptr);
  m_ImGuiLayer.Shutdown();
  m_VulkPipeline.reset();
  m_VulkDevice->Cleanup();
  m_Window->Shutdown();
  Log::Shutdown();
}

void Engine::Run()
{
  try
  {
    while (!m_Window->ShouldClose())
    {
      m_Window->OnUpdate();
      DrawFrame();
      m_ImGuiLayer.OnUpdate();
    }
  } catch (const std::exception& e)
  {
    LOG_CORE_ERROR("Exception: {}", e.what());
  }

  vkDeviceWaitIdle(m_VulkDevice->GetDevice());
}

bool Engine::InitLog()
{
  return Log::Init();
}

bool Engine::InitWindow(std::string_view title, uint32_t width, uint32_t height)
{
  m_Window.reset();
  if (m_Window = Window<UsingWindow>::CreateWindow(title, width, height); m_Window == nullptr)
  {
    LOG_ERROR("Initializing Window falied");
    return false;
  }

  // setup required events
  return true;
}

void Engine::OnResize(uint32_t width, uint32_t height)
{
  LOG_CORE_INFO("resize: w: {}, h {}", width, height);
}

void Engine::Shutdown()
{
  sm_Instance.reset();
}

void Engine::CreatePipeLineLayout()
{
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount         = 0;
  pipelineLayoutInfo.pSetLayouts            = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges    = nullptr;
  if (vkCreatePipelineLayout(m_VulkDevice->GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
  {
    LOG_CORE_ERROR("Failed creating pipeline layout");
    throw std::runtime_error("failed creating pipeline layout");
  }
}
void Engine::CreatePipeLine()
{
  PipeLineConfigInfo pipelineConfig{};
  VulkPipeline::DefaultPipeLineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass     = m_SwapChain->GetRenderPass();
  pipelineConfig.pipelineLayout = m_PipelineLayout;
  m_VulkPipeline                = std::make_unique<VulkPipeline>(*m_VulkDevice,
                                                  pipelineConfig,
                                                  "shaders/simpleShader.vert.spv",
                                                  "shaders/simpleShader.frag.spv");
}

void Engine::ReCreateSwapChain()
{
  auto extent = m_Window->GetExtent();
  while (extent.width == 0 || extent.height == 0)
  {
    extent = m_Window->GetExtent();
    m_Window->WaitEvents();
  }

  vkDeviceWaitIdle(m_VulkDevice->GetDevice());

  // if already have a valid swapchain, pass it to VulkSwapChain
  if (m_SwapChain)
  {
    m_SwapChain = std::make_unique<VulkSwapChain>(*m_VulkDevice, extent, std::move(m_SwapChain));

    // if ImageCount is not same as CommandBuffers, free and create new
    if (m_SwapChain->ImageCount() != m_CommandBuffers.size())
    {
      FreeCommandBuffer();
      CreateCommandBuffers();
    }
  }
  else
  {
    m_SwapChain = std::make_unique<VulkSwapChain>(*m_VulkDevice, extent);
  }

  CreatePipeLine();
}


void Engine::FreeCommandBuffer()
{
  vkFreeCommandBuffers(m_VulkDevice->GetDevice(),
                       m_VulkDevice->GetCommandPool(),
                       static_cast<uint32_t>(m_CommandBuffers.size()),
                       m_CommandBuffers.data());

  m_CommandBuffers.clear();
}

void Engine::CreateCommandBuffers()
{
  m_CommandBuffers.resize(m_SwapChain->ImageCount());

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool        = m_VulkDevice->GetCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

  if (vkAllocateCommandBuffers(m_VulkDevice->GetDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
  {
    LOG_CORE_ERROR("failed to allocate command buffers!");
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

void Engine::RecordCommandBuffers(uint32_t imageIndex)
{
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(m_CommandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
  {
    LOG_CORE_ERROR("failed to begin recording command buffer!");
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass  = m_SwapChain->GetRenderPass();
  renderPassInfo.framebuffer = m_SwapChain->GetFrameBuffer(static_cast<int>(imageIndex));

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color           = {0.1F, 0.1F, 0.1F, 1.0F};
  clearValues[1].depthStencil    = {1.0F, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues    = clearValues.data();

  vkCmdBeginRenderPass(m_CommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x        = 0.0F;
  viewport.y        = 0.0F;
  viewport.width    = static_cast<float>(m_SwapChain->GetSwapChainExtent().width);
  viewport.height   = static_cast<float>(m_SwapChain->GetSwapChainExtent().height);
  viewport.minDepth = 0.0F;
  viewport.maxDepth = 1.0F;
  VkRect2D scissor{{0, 0}, m_SwapChain->GetSwapChainExtent()};
  vkCmdSetViewport(m_CommandBuffers[imageIndex], 0, 1, &viewport);
  vkCmdSetScissor(m_CommandBuffers[imageIndex], 0, 1, &scissor);

  m_VulkPipeline->Bind(m_CommandBuffers[imageIndex]);
  m_VulkModel->Bind(m_CommandBuffers[imageIndex]);
  m_VulkModel->Draw(m_CommandBuffers[imageIndex]);

  vkCmdEndRenderPass(m_CommandBuffers[imageIndex]);
  if (vkEndCommandBuffer(m_CommandBuffers[imageIndex]) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to record command buffer!");
  }
}
void Engine::DrawFrame()
{
  uint32_t imageIndex = 0;
  VkResult result     = m_SwapChain->AcquireNextImage(&imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    ReCreateSwapChain();
    return;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
  {
    throw std::runtime_error("failed to acquire swap chain image");
  }

  RecordCommandBuffers(imageIndex);
  result = m_SwapChain->SubmitCommandBuffers(&m_CommandBuffers[imageIndex], &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window->WasWindowResized())
  {
    m_Window->ResetWindowResized();
    ReCreateSwapChain();
    return;
  }
  else if (result != VK_SUCCESS)
  {
    throw std::runtime_error("failed to present swap chain image");
  }
}

void Engine::LoadModels()
{
  std::vector<VulkModel::Vertex> vertices = {{{0.0F, -0.5F}, {1.0F, 0.0F, 0.0F}},
                                             {{0.5F, 0.5F}, {0.0F, 1.0F, 0.0F}},
                                             {{-0.5F, 0.5F}, {0.0F, 0.0F, 1.0F}}};

  m_VulkModel = std::make_unique<VulkModel>(*m_VulkDevice, vertices);
}

} // namespace four
