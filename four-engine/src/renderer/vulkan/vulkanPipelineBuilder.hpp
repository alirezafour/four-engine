#pragma once
#include "core/core.hpp"
#include "vulkan/vulkan.hpp"

namespace four
{

class FOUR_ENGINE_API VulkanPipelineBuilder
{
public:
  VulkanPipelineBuilder()
  {
    clear();
  }

  std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
  vk::PipelineRasterizationStateCreateInfo rasterizer{};
  vk::PipelineColorBlendAttachmentState    colorBlendAttachment{};
  vk::PipelineMultisampleStateCreateInfo   multisampling{};
  vk::PipelineLayout                       pipelineLayout{nullptr};
  vk::PipelineDepthStencilStateCreateInfo  depthStencil{};
  vk::PipelineRenderingCreateInfo          renderInfo{};
  vk::Format                               colorAttachmentformat{};


  void clear();

  vk::Pipeline BuildPipeline(vk::Device device);

  VulkanPipelineBuilder& SetShaders(vk::ShaderModule vertexShader, vk::ShaderModule fragmentShader);
  VulkanPipelineBuilder& SetInputTopology(vk::PrimitiveTopology topology);
  VulkanPipelineBuilder& SetPolygonMode(vk::PolygonMode mode);
  VulkanPipelineBuilder& SetCullMode(vk::CullModeFlagBits cullMode, vk::FrontFace frontFace);
  VulkanPipelineBuilder& SetMultiSamplingNone();
  VulkanPipelineBuilder& DisableBlending();
  VulkanPipelineBuilder& SetColorAttachmentFormat(vk::Format format);
  VulkanPipelineBuilder& SetDepthFormat(vk::Format format);
  VulkanPipelineBuilder& DisableDepthTest();
};

} // namespace four
