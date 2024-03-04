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
  m_SwapChain = std::make_unique<VulkSwapChain>(*m_VulkDevice, m_Window->GetExtent());
  // m_VulkPipeline = std::make_unique<VulkPipeline>(*m_VulkDevice,
  //                                                 VulkPipeline::DefaultPipeLineConfigInfo(width, height),
  //                                                 "shaders/simpleShader.vert.spv",
  //                                                 "shaders/simpleShader.frag.spv");

  LoadModels();
  CreatePipeLineLayout();
  CreatePipeLine();
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
  while (!m_Window->ShouldClose())
  {
    m_Window->OnUpdate();
    DrawFrame();
    m_ImGuiLayer.OnUpdate();
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
  auto pipelineConfig = VulkPipeline::DefaultPipeLineConfigInfo(m_SwapChain->GetWidth(), m_SwapChain->GetHeight());
  pipelineConfig.renderPass     = m_SwapChain->GetRenderPass();
  pipelineConfig.pipelineLayout = m_PipelineLayout;
  m_VulkPipeline                = std::make_unique<VulkPipeline>(*m_VulkDevice,
                                                  pipelineConfig,
                                                  "shaders/simpleShader.vert.spv",
                                                  "shaders/simpleShader.frag.spv");
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

  for (int i = 0; i < m_CommandBuffers.size(); ++i)
  {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo) != VK_SUCCESS)
    {
      LOG_CORE_ERROR("failed to begin recording command buffer!");
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass  = m_SwapChain->GetRenderPass();
    renderPassInfo.framebuffer = m_SwapChain->GetFrameBuffer(i);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color           = {0.1f, 0.1f, 0.1f, 1.0f};
    clearValues[1].depthStencil    = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues    = clearValues.data();

    vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    m_VulkPipeline->Bind(m_CommandBuffers[i]);
    m_VulkModel->Bind(m_CommandBuffers[i]);
    m_VulkModel->Draw(m_CommandBuffers[i]);

    vkCmdEndRenderPass(m_CommandBuffers[i]);
    if (vkEndCommandBuffer(m_CommandBuffers[i]) != VK_SUCCESS)
    {
      LOG_CORE_ERROR("failed to record command buffer!");
      throw std::runtime_error("failed to record command buffer!");
    }
  }
}

void Engine::DrawFrame()
{
  uint32_t imageIndex = 0;
  VkResult result     = m_SwapChain->AcquireNextImage(&imageIndex);
  if (result != VK_SUCCESS &&
      result != VK_SUBOPTIMAL_KHR) // TODO: Should be handle properly in production ( in resize it throw error)
  {
    LOG_CORE_ERROR("failed to acquire swap chain image");
    throw std::runtime_error("failed to acquire swap chain image");
  }
  result = m_SwapChain->SubmitCommandBuffers(&m_CommandBuffers[imageIndex], &imageIndex);
  if (result != VK_SUCCESS)
  {
    LOG_CORE_ERROR("failed to present swap chain image");
    throw std::runtime_error("failed to present swap chain image");
  }
}

void Engine::LoadModels()
{
  std::vector<VulkModel::Vertex> vertices = {
    {{0.0F, -0.5F}},
    {{0.5F, 0.5F}},
    {{-0.5F, 0.5F}},
  };

  m_VulkModel = std::make_unique<VulkModel>(*m_VulkDevice, vertices);
}

} // namespace four
