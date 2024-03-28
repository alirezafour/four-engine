#pragma once

#include "vulkan/vulkan.hpp"

namespace four
{
class VulkDevice;

struct PipeLineConfigInfo
{
  PipeLineConfigInfo()                                     = default;
  PipeLineConfigInfo(const PipeLineConfigInfo&)            = delete;
  PipeLineConfigInfo& operator=(const PipeLineConfigInfo&) = delete;

  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  vk::PipelineViewportStateCreateInfo      viewportInfo;
  vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
  vk::PipelineMultisampleStateCreateInfo   multisampleInfo;
  vk::PipelineColorBlendAttachmentState    colorBlendAttachment;
  vk::PipelineColorBlendStateCreateInfo    colorBlendInfo;
  vk::PipelineDepthStencilStateCreateInfo  depthStencilInfo;
  vk::PipelineDynamicStateCreateInfo       dynamicStateInfo;
  vk::PipelineLayout                       pipelineLayout = nullptr;
  vk::RenderPass                           renderPass     = nullptr;
  uint32_t                                 subpass        = 0;
};

class VulkPipeline
{
public:
  explicit VulkPipeline(VulkDevice& device, const PipeLineConfigInfo& config, std::string_view vertPath, std::string_view fragPath);
  VulkPipeline(const VulkPipeline&)            = delete;
  VulkPipeline(VulkPipeline&&)                 = delete;
  VulkPipeline& operator=(const VulkPipeline&) = delete;
  VulkPipeline& operator=(VulkPipeline&&)      = delete;
  ~VulkPipeline();

  static void DefaultPipeLineConfigInfo(PipeLineConfigInfo& configInfo);

  void Bind(vk::CommandBuffer commandBuffer);

private:
  static std::vector<char> ReadFile(std::string_view filePath);

  void CreateGraphicPipeline(std::string_view vertPath, std::string_view fragPath, const PipeLineConfigInfo& configInfo);

  [[nodiscard]] vk::ShaderModule CreateShaderModule(const std::vector<char>& code);

private:
  VulkDevice&      m_Device;
  vk::Pipeline     m_GraphicsPipeline;
  vk::ShaderModule m_VertShaderModule;
  vk::ShaderModule m_FragShaderModule;
};


} // namespace four
