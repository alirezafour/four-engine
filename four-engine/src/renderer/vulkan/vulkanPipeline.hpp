#pragma once

#include "core/core.hpp"

#include "vulkan/vulkan.hpp"

namespace four
{

class FOUR_ENGINE_API VulkanPipeline
{
public:
  explicit VulkanPipeline(vk::Device& device, vk::Extent2D extent);
  ~VulkanPipeline();

  VulkanPipeline(const VulkanPipeline&)            = delete;
  VulkanPipeline& operator=(const VulkanPipeline&) = delete;
  VulkanPipeline(VulkanPipeline&&)                 = delete;
  VulkanPipeline& operator=(VulkanPipeline&&)      = delete;

private:
  // graphic pipeline
  [[nodiscard]] bool             CreateGraphicsPipeline();
  [[nodiscard]] vk::ShaderModule CreateShaderModule(const std::vector<char>& code);

private:
  vk::Device&        m_Device;
  vk::Extent2D       m_Extent;
  vk::RenderPass     m_RenderPass;
  vk::Pipeline       m_GraphicsPipeline;
  vk::ShaderModule   m_VertexShaderModule;
  vk::ShaderModule   m_FragmentShaderModule;
  vk::PipelineLayout m_PipelineLayout;
};

} // namespace four
