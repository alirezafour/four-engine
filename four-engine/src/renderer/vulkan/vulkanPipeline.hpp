#pragma once

#include "core/core.hpp"

#include "renderer/vulkan/vulkanContext.hpp"


namespace four
{

class FOUR_ENGINE_API VulkanPipeline
{
public:
  explicit VulkanPipeline(const VulkanContext& context);
  ~VulkanPipeline();

  VulkanPipeline(const VulkanPipeline&)            = delete;
  VulkanPipeline& operator=(const VulkanPipeline&) = delete;
  VulkanPipeline(VulkanPipeline&&)                 = delete;
  VulkanPipeline& operator=(VulkanPipeline&&)      = delete;


private:
  [[nodiscard]] bool Init();
  void               Shutdown();

  // graphic pipeline
  [[nodiscard]] bool             CreateGraphicsPipeline();
  [[nodiscard]] vk::ShaderModule CreateShaderModule(const std::vector<char>& code);

private:
  const vk::Device&  m_Device;
  vk::Extent2D       m_Extent;
  vk::RenderPass     m_RenderPass;
  vk::Pipeline       m_GraphicsPipeline;
  vk::ShaderModule   m_VertexShaderModule;
  vk::ShaderModule   m_FragmentShaderModule;
  vk::PipelineLayout m_PipelineLayout;
};

} // namespace four
