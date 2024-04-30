#include "four-pch.h"

#include "renderer/vulkan/vulkanPipeline.hpp"

#include <fstream>

namespace four
{

std::vector<char> ReadFile(const std::string& filename)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open())
  {
    throw std::runtime_error("failed to open file!");
  }
  const size_t      fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();
  return buffer;
}

//===============================================================================
VulkanPipeline::VulkanPipeline(VulkanRenderer& renderer) : m_VulkanRenderer{renderer}
{
  bool result = CreateGraphicsPipeline();
}
//===============================================================================
VulkanPipeline::~VulkanPipeline()
{
  const auto device = m_VulkanRenderer.GetDevice();

  device.destroyShaderModule(m_FragmentShaderModule);
  device.destroyShaderModule(m_VertexShaderModule);
}

//===============================================================================
bool VulkanPipeline::CreateGraphicsPipeline()
{
  const auto extent = m_VulkanRenderer.GetExtent();

  auto verShaderCode  = ReadFile("shaders/SimpleShader.vert.spv");
  auto fragShaderCode = ReadFile("shaders/SimpleShader.frag.spv");

  auto vertShaderModule = CreateShaderModule(verShaderCode);
  auto fragShaderModule = CreateShaderModule(fragShaderCode);

  vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType  = vk::StructureType::ePipelineShaderStageCreateInfo;
  vertShaderStageInfo.stage  = vk::ShaderStageFlagBits::eVertex;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName  = "main";

  vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType  = vk::StructureType::ePipelineShaderStageCreateInfo;
  fragShaderStageInfo.stage  = vk::ShaderStageFlagBits::eFragment;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName  = "main";

  std::array shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

  std::vector dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

  vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
  dynamicStateInfo.sType             = vk::StructureType::ePipelineDynamicStateCreateInfo;
  dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicStateInfo.pDynamicStates    = dynamicStates.data();


  vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType                           = vk::StructureType::ePipelineVertexInputStateCreateInfo;
  vertexInputInfo.vertexBindingDescriptionCount   = 0;
  vertexInputInfo.pVertexBindingDescriptions      = nullptr; // Optional
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions    = nullptr; // Optional

  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
  inputAssemblyInfo.sType                  = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
  inputAssemblyInfo.topology               = vk::PrimitiveTopology::eTriangleList;
  inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

  vk::Viewport viewport{};
  viewport.x        = 0.0f;
  viewport.y        = 0.0f;
  viewport.width    = static_cast<float>(extent.width);
  viewport.height   = static_cast<float>(extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vk::Rect2D scissor{};
  scissor.offset = vk::Offset2D{0, 0};
  scissor.extent = extent;

  vk::PipelineViewportStateCreateInfo viewportStateInfo{};
  viewportStateInfo.sType         = vk::StructureType::ePipelineViewportStateCreateInfo;
  viewportStateInfo.viewportCount = 1;
  viewportStateInfo.pViewports    = &viewport;
  viewportStateInfo.scissorCount  = 1;
  viewportStateInfo.pScissors     = &scissor;

  vk::PipelineRasterizationStateCreateInfo rasterizerInfo{};
  rasterizerInfo.sType                   = vk::StructureType::ePipelineRasterizationStateCreateInfo;
  rasterizerInfo.depthClampEnable        = VK_FALSE;
  rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
  rasterizerInfo.polygonMode             = vk::PolygonMode::eFill;
  rasterizerInfo.lineWidth               = 1.0f;
  rasterizerInfo.cullMode                = vk::CullModeFlagBits::eBack;
  rasterizerInfo.frontFace               = vk::FrontFace::eCounterClockwise;
  rasterizerInfo.depthBiasEnable         = VK_FALSE;
  rasterizerInfo.depthBiasConstantFactor = 0.0f; // Optional
  rasterizerInfo.depthBiasClamp          = 0.0f; // Optional
  rasterizerInfo.depthBiasSlopeFactor    = 0.0f; // Optional

  vk::PipelineMultisampleStateCreateInfo multisamplingInfo{};
  multisamplingInfo.sType                 = vk::StructureType::ePipelineMultisampleStateCreateInfo;
  multisamplingInfo.sampleShadingEnable   = VK_FALSE;
  multisamplingInfo.rasterizationSamples  = vk::SampleCountFlagBits::e1;
  multisamplingInfo.minSampleShading      = 1.0f;     // Optional
  multisamplingInfo.pSampleMask           = nullptr;  // Optional
  multisamplingInfo.alphaToCoverageEnable = VK_FALSE; // Optional
  multisamplingInfo.alphaToOneEnable      = VK_FALSE; // Optional

  vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  colorBlendAttachment.blendEnable         = VK_TRUE;
  colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
  colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
  colorBlendAttachment.colorBlendOp        = vk::BlendOp::eAdd;
  colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
  colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
  colorBlendAttachment.alphaBlendOp        = vk::BlendOp::eAdd;

  vk::PipelineColorBlendStateCreateInfo colorBlendingInfo{};
  colorBlendingInfo.sType             = vk::StructureType::ePipelineColorBlendStateCreateInfo;
  colorBlendingInfo.logicOpEnable     = VK_FALSE;
  colorBlendingInfo.logicOp           = vk::LogicOp::eCopy;
  colorBlendingInfo.attachmentCount   = 1;
  colorBlendingInfo.pAttachments      = &colorBlendAttachment;
  colorBlendingInfo.blendConstants[0] = 0.0f; // Optional
  colorBlendingInfo.blendConstants[1] = 0.0f; // Optional
  colorBlendingInfo.blendConstants[2] = 0.0f; // Optional
  colorBlendingInfo.blendConstants[3] = 0.0f; // Optional

  return true;
}

//===============================================================================
vk::ShaderModule VulkanPipeline::CreateShaderModule(const std::vector<char>& code)
{
  const auto                 device = m_VulkanRenderer.GetDevice();
  vk::ShaderModuleCreateInfo createInfo{};
  createInfo.sType    = vk::StructureType::eShaderModuleCreateInfo;
  createInfo.codeSize = code.size();
  createInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());
  return device.createShaderModule(createInfo);
}
} // namespace four
