#include "four-pch.h"

#include "renderer/simpleRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

namespace four
{

// testing push constants
struct SimplePushConstants
{
  glm::mat2 transform{1.0F};
  glm::vec2 offset;
  alignas(16) glm::vec3 color;
};


//====================================================================================================
SimpleRenderSystem::SimpleRenderSystem(VulkDevice& vulkDevice, VkRenderPass renderPass) : m_VulkDevice{vulkDevice}
{
  CreatePipeLineLayout();
  CreatePipeLine(renderPass);
}

//====================================================================================================
SimpleRenderSystem::~SimpleRenderSystem()
{
  vkDestroyPipelineLayout(m_VulkDevice.GetDevice(), m_PipelineLayout, nullptr);
}


//====================================================================================================
void SimpleRenderSystem::CreatePipeLineLayout()
{

  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset     = 0;
  pushConstantRange.size       = sizeof(SimplePushConstants);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount         = 0;
  pipelineLayoutInfo.pSetLayouts            = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges    = &pushConstantRange;
  if (vkCreatePipelineLayout(m_VulkDevice.GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed creating pipeline layout");
  }
}

//====================================================================================================
void SimpleRenderSystem::CreatePipeLine(VkRenderPass renderPass)
{
  assert(m_PipelineLayout != nullptr && "Cannot create pipeline without pipeline layout");

  PipeLineConfigInfo pipelineConfig{};
  VulkPipeline::DefaultPipeLineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass     = renderPass;
  pipelineConfig.pipelineLayout = m_PipelineLayout;
  m_VulkPipeline                = std::make_unique<VulkPipeline>(m_VulkDevice,
                                                  pipelineConfig,
                                                  "shaders/simpleShader.vert.spv",
                                                  "shaders/simpleShader.frag.spv");
}

//====================================================================================================
void SimpleRenderSystem::RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<TempGameObj>& gameObjects, float deltaTime)
{
  m_VulkPipeline->Bind(commandBuffer);
  for (auto& gameObj : gameObjects)
  {
    auto transform     = gameObj.GetTransform2D();
    transform.rotation = glm::mod(transform.rotation + (0.5F * deltaTime), glm::two_pi<float>());
    gameObj.SetTransform2D(transform);
    SimplePushConstants push{};
    push.offset    = gameObj.GetTransform2D().translation;
    push.color     = gameObj.GetColor();
    push.transform = gameObj.GetTransform2D().mat2();
    vkCmdPushConstants(commandBuffer,
                       m_PipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       0,
                       sizeof(SimplePushConstants),
                       &push);
    gameObj.GetModel()->Bind(commandBuffer);
    gameObj.GetModel()->Draw(commandBuffer);
  }
}

} // namespace four
