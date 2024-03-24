#include "four-pch.h"

#include "renderer/vulkan/vulkDevice.hpp"
#include "renderer/vulkan/vulkPipline.hpp"
#include "renderer/vulkan/vulkModel.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"

#include <fstream>
#include <stdexcept>

namespace four
{

VulkPipeline::VulkPipeline(VulkDevice&               device,
                           const PipeLineConfigInfo& config,
                           std::string_view          vertPath,
                           std::string_view          fragPath) :
m_Device{device}
{
  CreateGraphicPipeline(vertPath, fragPath, config);
}

VulkPipeline::~VulkPipeline()
{
  auto device = m_Device.GetDevice();
  device.destroyPipeline(m_GraphicsPipeline);
  device.destroyShaderModule(m_VertShaderModule);
  device.destroyShaderModule(m_FragShaderModule);
}

void VulkPipeline::Bind(vk::CommandBuffer commandBuffer)
{
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline);
}
std::vector<char> VulkPipeline::ReadFile(std::string_view filePath)
{
  std::ifstream file(filePath.data(), std::ios::ate | std::ios::binary);
  if (!file.is_open())
  {
    throw std::runtime_error("failed to open file: " + std::string(filePath));
  }

  std::size_t       fileSize = static_cast<std::size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

void VulkPipeline::CreateGraphicPipeline(std::string_view vertPath, std::string_view fragPath, const PipeLineConfigInfo& configInfo)
{
  assert(configInfo.pipelineLayout != VK_NULL_HANDLE &&
         "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
  assert(configInfo.renderPass != VK_NULL_HANDLE &&
         "Cannot create graphics pipeline: no renderPass provided in configInfo");

  auto vertCode = ReadFile(vertPath);
  auto fragCode = ReadFile(fragPath);

  m_VertShaderModule = CreateShaderModule(vertCode);
  m_FragShaderModule = CreateShaderModule(fragCode);

  std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages{
    {vk::PipelineShaderStageCreateInfo{vk::PipelineShaderStageCreateFlags{},
                                       vk::ShaderStageFlagBits::eVertex,
                                       m_VertShaderModule,
                                       "main",
                                       nullptr,
                                       nullptr},
     vk::PipelineShaderStageCreateInfo{vk::PipelineShaderStageCreateFlags{},
                                       vk::ShaderStageFlagBits::eFragment,
                                       m_FragShaderModule,
                                       "main",
                                       nullptr,
                                       nullptr}}};

  auto bindingDescriptions   = VulkModel::Vertex::GetBindingDescriptions();
  auto attributeDescriptions = VulkModel::Vertex::GetAttributeDescriptions();

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
  vertexInputInfo.sType                           = vk::StructureType::ePipelineVertexInputStateCreateInfo;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.vertexBindingDescriptionCount   = static_cast<uint32_t>(bindingDescriptions.size());

  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
  vertexInputInfo.pVertexBindingDescriptions   = bindingDescriptions.data();

  vk::GraphicsPipelineCreateInfo pipelineInfo{
    vk::PipelineCreateFlags{},                  // flags
    static_cast<uint32_t>(shaderStages.size()), //stages count
    shaderStages.data(),                        // stages
    &vertexInputInfo,                           // vertex input state
    &configInfo.inputAssemblyInfo,              // input assembly state
    {},                                         // tessellation state
    &configInfo.viewportInfo,                   // viewport state
    &configInfo.rasterizationInfo,              // rasterization state
    &configInfo.multisampleInfo,                // multisample state
    &configInfo.depthStencilInfo,               // depth stencil state
    &configInfo.colorBlendInfo,                 // color blend state
    &configInfo.dynamicStateInfo,               // dynamic state
    configInfo.pipelineLayout,                  // layout
    configInfo.renderPass,                      // render pass
    configInfo.subpass,                         // subpass
    VK_NULL_HANDLE,                             // base pipeline handle
    -1                                          // base pipeline index
  };


  if (vk::Result result = m_Device.GetDevice().createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline);
      result != vk::Result::eSuccess)
  {
    throw std::runtime_error("failed to create graphics pipeline");
  }
}


vk::ShaderModule VulkPipeline::CreateShaderModule(const std::vector<char>& code)
{
  vk::ShaderModuleCreateInfo createInfo{
    vk::ShaderModuleCreateFlags{},                 // flags
    static_cast<uint32_t>(code.size()),            // code size
    reinterpret_cast<const uint32_t*>(code.data()) // code
  };

  auto shaderModule = m_Device.GetDevice().createShaderModule(createInfo, nullptr);
  if (!shaderModule)
  {
    throw std::runtime_error("failed to create shader module");
  }
  return shaderModule;
}


void VulkPipeline::DefaultPipeLineConfigInfo(PipeLineConfigInfo& configInfo)
{

  configInfo.inputAssemblyInfo.sType                  = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
  configInfo.inputAssemblyInfo.topology               = vk::PrimitiveTopology::eTriangleList;
  configInfo.inputAssemblyInfo.primitiveRestartEnable = vk::Bool32(false);

  configInfo.viewportInfo.sType         = vk::StructureType::ePipelineViewportStateCreateInfo;
  configInfo.viewportInfo.viewportCount = 1;
  configInfo.viewportInfo.pViewports    = nullptr;
  configInfo.viewportInfo.scissorCount  = 1;
  configInfo.viewportInfo.pScissors     = nullptr;

  configInfo.rasterizationInfo.sType                   = vk::StructureType::ePipelineRasterizationStateCreateInfo;
  configInfo.rasterizationInfo.depthClampEnable        = vk::Bool32(false);
  configInfo.rasterizationInfo.rasterizerDiscardEnable = vk::Bool32(false);
  configInfo.rasterizationInfo.polygonMode             = vk::PolygonMode::eFill;
  configInfo.rasterizationInfo.lineWidth               = 1.0F;
  configInfo.rasterizationInfo.cullMode                = vk::CullModeFlagBits::eNone;
  configInfo.rasterizationInfo.frontFace               = vk::FrontFace::eCounterClockwise;
  configInfo.rasterizationInfo.depthBiasEnable         = vk::Bool32(false);
  configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0F; // Optional
  configInfo.rasterizationInfo.depthBiasClamp          = 0.0F; // Optional
  configInfo.rasterizationInfo.depthBiasSlopeFactor    = 0.0F; // Optional

  configInfo.multisampleInfo.sType                 = vk::StructureType::ePipelineMultisampleStateCreateInfo;
  configInfo.multisampleInfo.sampleShadingEnable   = vk::Bool32(false);
  configInfo.multisampleInfo.rasterizationSamples  = vk::SampleCountFlagBits::e1;
  configInfo.multisampleInfo.minSampleShading      = 1.0F;              // Optional
  configInfo.multisampleInfo.pSampleMask           = nullptr;           // Optional
  configInfo.multisampleInfo.alphaToCoverageEnable = vk::Bool32(false); // Optional
  configInfo.multisampleInfo.alphaToOneEnable      = vk::Bool32(false); // Optional

  configInfo.colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                                   vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  configInfo.colorBlendAttachment.blendEnable         = vk::Bool32(false);
  configInfo.colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;  // Optional
  configInfo.colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero; // Optional
  configInfo.colorBlendAttachment.colorBlendOp        = vk::BlendOp::eAdd;      // Optional
  configInfo.colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;  // Optional
  configInfo.colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero; // Optional
  configInfo.colorBlendAttachment.alphaBlendOp        = vk::BlendOp::eAdd;      // Optional

  configInfo.colorBlendInfo.sType             = vk::StructureType::ePipelineColorBlendStateCreateInfo;
  configInfo.colorBlendInfo.logicOpEnable     = vk::Bool32(false);
  configInfo.colorBlendInfo.logicOp           = vk::LogicOp::eCopy; // Optional
  configInfo.colorBlendInfo.attachmentCount   = 1;
  configInfo.colorBlendInfo.pAttachments      = &configInfo.colorBlendAttachment;
  configInfo.colorBlendInfo.blendConstants[0] = 0.0F; // Optional
  configInfo.colorBlendInfo.blendConstants[1] = 0.0F; // Optional
  configInfo.colorBlendInfo.blendConstants[2] = 0.0F; // Optional
  configInfo.colorBlendInfo.blendConstants[3] = 0.0F; // Optional

  configInfo.depthStencilInfo.sType                 = vk::StructureType::ePipelineDepthStencilStateCreateInfo;
  configInfo.depthStencilInfo.depthTestEnable       = vk::Bool32(true);
  configInfo.depthStencilInfo.depthWriteEnable      = vk::Bool32(true);
  configInfo.depthStencilInfo.depthCompareOp        = vk::CompareOp::eLess;
  configInfo.depthStencilInfo.depthBoundsTestEnable = vk::Bool32(false);
  configInfo.depthStencilInfo.minDepthBounds        = 0.0F; // Optional
  configInfo.depthStencilInfo.maxDepthBounds        = 1.0F; // Optional
  configInfo.depthStencilInfo.stencilTestEnable     = vk::Bool32(false);
  configInfo.depthStencilInfo.front                 = vk::StencilOpState{}; // Optional
  configInfo.depthStencilInfo.back                  = vk::StencilOpState{}; // Optional

  std::array<vk::DynamicState, 2>    dynamicState = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
  vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{
    vk::PipelineDynamicStateCreateFlags(), // flags
    dynamicState                           // enable sttates
  };

  configInfo.dynamicStateInfo = pipelineDynamicStateCreateInfo;
}
} // namespace four
