#pragma once

#include "vulkan/vulkan_core.h"

namespace four
{
class VulkDevice;

struct PipeLineConfigInfo
{
  PipeLineConfigInfo()                                     = default;
  PipeLineConfigInfo(const PipeLineConfigInfo&)            = delete;
  PipeLineConfigInfo& operator=(const PipeLineConfigInfo&) = delete;

  VkPipelineViewportStateCreateInfo      viewportInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo   multisampleInfo;
  VkPipelineColorBlendAttachmentState    colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo    colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo  depthStencilInfo;
  std::vector<VkDynamicState>            dynamicStateEnables;
  VkPipelineDynamicStateCreateInfo       dynamicStateInfo;
  VkPipelineLayout                       pipelineLayout = nullptr;
  VkRenderPass                           renderPass     = nullptr;
  uint32_t                               subpass        = 0;
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

  void Bind(VkCommandBuffer commandBuffer);

private:
  static std::vector<char> ReadFile(std::string_view filePath);

  void CreateGraphicPipeline(std::string_view vertPath, std::string_view fragPath, const PipeLineConfigInfo& configInfo);

  void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

private:
  VulkDevice&    m_Device;
  VkPipeline     m_GraphicsPipeline;
  VkShaderModule m_VertShaderModule;
  VkShaderModule m_FragShaderModule;
};


} // namespace four
