#pragma once

#include "core/core.hpp"

#include "vulkan/vulkan.hpp"

namespace four
{

class FOUR_ENGINE_API VulkanPipeline
{
public:
  explicit VulkanPipeline(const vk::Device& device, const vk::Extent2D& extent, vk::Format swapChainImageFormat);
  ~VulkanPipeline();

  VulkanPipeline(const VulkanPipeline&)            = delete;
  VulkanPipeline& operator=(const VulkanPipeline&) = delete;
  VulkanPipeline(VulkanPipeline&&)                 = delete;
  VulkanPipeline& operator=(VulkanPipeline&&)      = delete;


private:
  [[nodiscard]] bool Init();
  void               Shutdown();

  // graphic pipeline
  [[nodiscard]] bool             CreateRenderPass();
  [[nodiscard]] bool             CreateGraphicsPipeline();
  [[nodiscard]] vk::ShaderModule CreateShaderModule(const std::vector<char>& code);
  [[nodiscard]] vk::Format       FindDepthFormat();

private:
  const vk::Device&  m_Device;
  const vk::Extent2D m_Extent;
  const vk::Format   m_SwapChainImageFormat;
  vk::RenderPass     m_RenderPass;
  vk::Pipeline       m_GraphicsPipeline;
  vk::ShaderModule   m_VertexShaderModule;
  vk::ShaderModule   m_FragmentShaderModule;
  vk::PipelineLayout m_PipelineLayout;
};

} // namespace four
