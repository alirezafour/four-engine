#pragma once

// test vulkan
#include "renderer/vulkan/vulkPipline.hpp"
#include "renderer/vulkan/vulkDevice.hpp"
#include "renderer/vulkan/vulkModel.hpp"

// test gameobject
#include "core/tempGameObj.hpp"

namespace four
{

class SimpleRenderSystem
{
public:
  explicit SimpleRenderSystem(VulkDevice& vulkDevice, VkRenderPass renderPass);
  ~SimpleRenderSystem();

  void RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<TempGameObj>& gameObjects, float deltaTime);

private:
  void CreatePipeLineLayout();
  void CreatePipeLine(VkRenderPass renderPass);

private:
  VulkDevice&                   m_VulkDevice;
  std::unique_ptr<VulkPipeline> m_VulkPipeline;
  VkPipelineLayout              m_PipelineLayout;
};
} // namespace four
