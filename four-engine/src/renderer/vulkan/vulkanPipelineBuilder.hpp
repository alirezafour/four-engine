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

  vk::Pipeline build_pipeline(vk::Device device);

  void SetShaders(vk::ShaderModule vertexShader, vk::ShaderModule fragmentShader);
  void SetInputTopology(vk::PrimitiveTopology topology);
  void SetPolygonMode(vk::PolygonMode mode);
  void SetCullMode(vk::CullModeFlagBits cullMode, vk::FrontFace frontFace);
  void SetMultiSamplingNone();
  void DisableBlenging();
  void SetColorAttachmentFormat(vk::Format format);
  void SetDepthFormat(vk::Format format);
  void DisableDepthTest();
};

} // namespace four
