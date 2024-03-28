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
SimpleRenderSystem::SimpleRenderSystem(VulkDevice& vulkDevice, vk::RenderPass renderPass) : m_VulkDevice{vulkDevice}
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

  vk::PushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
  pushConstantRange.offset     = 0;
  pushConstantRange.size       = sizeof(SimplePushConstants);

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType                  = vk::StructureType::ePipelineLayoutCreateInfo;
  pipelineLayoutInfo.setLayoutCount         = 0;
  pipelineLayoutInfo.pSetLayouts            = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges    = &pushConstantRange;

  m_PipelineLayout = m_VulkDevice.GetDevice().createPipelineLayout(pipelineLayoutInfo, nullptr);
}

//====================================================================================================
void SimpleRenderSystem::CreatePipeLine(vk::RenderPass renderPass)
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
void SimpleRenderSystem::RenderGameObjects(vk::CommandBuffer commandBuffer, std::vector<TempGameObj>& gameObjects, float deltaTime)
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

    commandBuffer.pushConstants(m_PipelineLayout,
                                vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                                0,
                                sizeof(SimplePushConstants),
                                &push);
    gameObj.GetModel()->Bind(commandBuffer);
    gameObj.GetModel()->Draw(commandBuffer);
  }
}

} // namespace four
