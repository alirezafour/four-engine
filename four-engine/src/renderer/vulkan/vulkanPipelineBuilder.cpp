#include "vulkanPipelineBuilder.hpp"


namespace four
{

//==============================================================================
void VulkanPipelineBuilder::clear()
{
  inputAssembly         = {};
  rasterizer            = {};
  colorBlendAttachment  = {};
  multisampling         = {};
  pipelineLayout        = nullptr;
  depthStencil          = {};
  renderInfo            = {};
  colorAttachmentformat = {};

  shaderStages.clear();
}

//==============================================================================
vk::Pipeline VulkanPipelineBuilder::BuildPipeline(vk::Device device)
{
  // make viewport state from our stored viewport and scissor.
  // at the moment we wont support multiple viewports or scissors
  vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1, .scissorCount = 1};

  // setup dummy color blending. We arent using transparent objects yet
  // the blending is just "no blend", but we do write to the color attachment
  vk::PipelineColorBlendStateCreateInfo colorBlending{
    .logicOpEnable   = VK_FALSE,
    .logicOp         = vk::LogicOp::eCopy,
    .attachmentCount = 1,
    .pAttachments    = &colorBlendAttachment,
  };

  // completely clear VertexInputStateCreateInfo, as we have no need for it
  vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};

  // build the actual pipeline
  // we now use all of the info structs we have been writing into into this one
  // to create the pipeline
  //
  // prepare dytamic state first
  std::array state = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

  vk::PipelineDynamicStateCreateInfo dynamicInfo = {
    .dynamicStateCount = 2,
    .pDynamicStates    = state.data(),
  };

  vk::GraphicsPipelineCreateInfo pipelineInfo{
    .pNext               = &renderInfo, // connect the renderInfo to the pNext extension mechanism
    .stageCount          = static_cast<uint32_t>(shaderStages.size()),
    .pStages             = shaderStages.data(),
    .pVertexInputState   = &vertexInputInfo,
    .pInputAssemblyState = &inputAssembly,
    .pViewportState      = &viewportState,
    .pRasterizationState = &rasterizer,
    .pMultisampleState   = &multisampling,
    .pDepthStencilState  = &depthStencil,
    .pColorBlendState    = &colorBlending,
    .pDynamicState       = &dynamicInfo,
    .layout              = pipelineLayout,
  };

  // its easy to error out on create graphics pipeline, so we handle it a bit
  // better than the common VK_CHECK case
  vk::Pipeline newPipeline;
  auto         result = device.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline);
  if (result != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to create graphics pipeline");
    return VK_NULL_HANDLE; // failed to create graphics pipeline
  }

  return newPipeline;
}

//==============================================================================
VulkanPipelineBuilder& VulkanPipelineBuilder::SetShaders(vk::ShaderModule vertexShader, vk::ShaderModule fragmentShader)
{
  shaderStages.clear();

  shaderStages.push_back({.stage = vk::ShaderStageFlagBits::eVertex, .module = vertexShader, .pName = "main"});
  shaderStages.push_back({.stage = vk::ShaderStageFlagBits::eFragment, .module = fragmentShader, .pName = "main"});
  return *this;
}

//==============================================================================
VulkanPipelineBuilder& VulkanPipelineBuilder::SetInputTopology(vk::PrimitiveTopology topology)
{
  inputAssembly.topology = topology;

  // we are not going to use primitive restart on the entire tutorial so leave
  // it on false
  inputAssembly.primitiveRestartEnable = VK_FALSE;
  return *this;
}
//==============================================================================
VulkanPipelineBuilder& VulkanPipelineBuilder::SetPolygonMode(vk::PolygonMode mode)
{
  rasterizer.polygonMode = mode;
  rasterizer.lineWidth   = 1.0F;
  return *this;
}

//==============================================================================
VulkanPipelineBuilder& VulkanPipelineBuilder::SetCullMode(vk::CullModeFlagBits cullMode, vk::FrontFace frontFace)
{
  rasterizer.cullMode  = cullMode;
  rasterizer.frontFace = frontFace;
  return *this;
}

//==============================================================================
VulkanPipelineBuilder& VulkanPipelineBuilder::SetMultiSamplingNone()
{
  multisampling.sampleShadingEnable = VK_FALSE;
  // multisampling defaulted to no multisampling (1 sample per pixel)
  multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
  multisampling.minSampleShading     = 1.0F;
  multisampling.pSampleMask          = nullptr;
  // no alpha to convert either
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable      = VK_FALSE;
  return *this;
}
//==============================================================================
VulkanPipelineBuilder& VulkanPipelineBuilder::DisableBlending()
{
  // defaults write mask
  colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  // no blending
  colorBlendAttachment.blendEnable = VK_FALSE;
  return *this;
}

//==============================================================================
VulkanPipelineBuilder& VulkanPipelineBuilder::SetColorAttachmentFormat(vk::Format format)
{
  colorAttachmentformat = format;
  // connect the format to the renderInfo struct
  renderInfo.colorAttachmentCount    = 1;
  renderInfo.pColorAttachmentFormats = &colorAttachmentformat;
  return *this;
}

//==============================================================================
VulkanPipelineBuilder& VulkanPipelineBuilder::SetDepthFormat(vk::Format format)
{
  renderInfo.depthAttachmentFormat = format;
  return *this;
}

//==============================================================================
VulkanPipelineBuilder& VulkanPipelineBuilder::DisableDepthTest()
{
  depthStencil.depthWriteEnable      = VK_FALSE;
  depthStencil.depthTestEnable       = VK_FALSE;
  depthStencil.depthCompareOp        = vk::CompareOp::eNever;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.stencilTestEnable     = VK_FALSE;
  depthStencil.front                 = {};
  depthStencil.back                  = {};
  depthStencil.minDepthBounds        = 0.0F;
  depthStencil.maxDepthBounds        = 1.0F;
  return *this;
}
//==============================================================================

} // namespace four
