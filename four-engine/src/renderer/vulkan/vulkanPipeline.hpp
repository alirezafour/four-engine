#pragma once

#include "core/core.hpp"

#include "renderer/vulkan/vulkanRenderer.hpp"

namespace four
{

class FOUR_ENGINE_API VulkanPipeline
{
public:
  explicit VulkanPipeline(VulkanRenderer& renderer);
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
  VulkanRenderer&  m_VulkanRenderer;
  vk::Pipeline     m_GraphicsPipeline;
  vk::ShaderModule m_VertexShaderModule;
  vk::ShaderModule m_FragmentShaderModule;
};

} // namespace four
