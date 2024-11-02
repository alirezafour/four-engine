#include "four-pch.h"

#include "renderer/vulkan/vulkanPipeline.hpp"

#include <fstream>

namespace four
{

//===============================================================================
namespace test
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
  file.read(buffer.data(), static_cast<long long>(fileSize));
  file.close();
  return buffer;
}
} // namespace test

//===============================================================================
VulkanPipeline::VulkanPipeline(const vk::Device& device, const vk::Extent2D& extent, vk::Format swapChainImageFormat) :
m_Device{device},
m_Extent{extent},
m_SwapChainImageFormat{swapChainImageFormat}
{
  const bool result = Init();
  if (!result)
  {
    LOG_CORE_ERROR("failed to create vulkan pipeline!");
  }
}

//===============================================================================
VulkanPipeline::~VulkanPipeline()
{
  Shutdown();
}

//===============================================================================
bool VulkanPipeline::Init()
{
  return CreateRenderPass() && CreateGraphicsPipeline();
}

//===============================================================================
void VulkanPipeline::Shutdown()
{
  if (m_Device)
  {
    m_Device.destroyPipelineLayout(m_PipelineLayout);
    m_Device.destroyPipeline(m_GraphicsPipeline);
    m_Device.destroyRenderPass(m_RenderPass);
    m_Device.destroyShaderModule(m_FragmentShaderModule);
    m_Device.destroyShaderModule(m_VertexShaderModule);
  }
}

//===============================================================================
bool VulkanPipeline::CreateRenderPass()
{
  vk::AttachmentDescription depthAttachment{};
  depthAttachment.format         = FindDepthFormat();
  depthAttachment.samples        = vk::SampleCountFlagBits::e1;
  depthAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
  depthAttachment.storeOp        = vk::AttachmentStoreOp::eDontCare;
  depthAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
  depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  depthAttachment.initialLayout  = vk::ImageLayout::eUndefined;
  depthAttachment.finalLayout    = vk::ImageLayout::eDepthStencilAttachmentOptimal;
  vk::AttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment              = 1;
  depthAttachmentRef.layout                  = vk::ImageLayout::eDepthStencilAttachmentOptimal;
  vk::AttachmentDescription colorAttachment  = {};
  colorAttachment.format                     = m_SwapChainImageFormat;
  colorAttachment.samples                    = vk::SampleCountFlagBits::e1;
  colorAttachment.loadOp                     = vk::AttachmentLoadOp::eClear;
  colorAttachment.storeOp                    = vk::AttachmentStoreOp::eStore;
  colorAttachment.stencilStoreOp             = vk::AttachmentStoreOp::eDontCare;
  colorAttachment.stencilLoadOp              = vk::AttachmentLoadOp::eDontCare;
  colorAttachment.initialLayout              = vk::ImageLayout::eUndefined;
  colorAttachment.finalLayout                = vk::ImageLayout::ePresentSrcKHR;
  vk::AttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment              = 0;
  colorAttachmentRef.layout                  = vk::ImageLayout::eColorAttachmentOptimal;
  vk::SubpassDescription subpass             = {};
  subpass.pipelineBindPoint                  = vk::PipelineBindPoint::eGraphics;
  subpass.colorAttachmentCount               = 1;
  subpass.pColorAttachments                  = &colorAttachmentRef;
  subpass.pDepthStencilAttachment            = &depthAttachmentRef;
  vk::SubpassDependency dependency           = {};
  dependency.srcSubpass                      = vk::SubpassExternal;
  dependency.srcAccessMask                   = vk::AccessFlagBits::eNone;
  dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
  dependency.dstSubpass = 0;
  dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
  dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
  std::array<vk::AttachmentDescription, 2> attachments    = {colorAttachment, depthAttachment};
  vk::RenderPassCreateInfo                 renderPassInfo = {};
  renderPassInfo.sType                                    = vk::StructureType::eRenderPassCreateInfo;
  renderPassInfo.attachmentCount                          = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments                             = attachments.data();
  renderPassInfo.subpassCount                             = 1;
  renderPassInfo.pSubpasses                               = &subpass;
  renderPassInfo.dependencyCount                          = 1;
  renderPassInfo.pDependencies                            = &dependency;
  if (auto result = m_Device.createRenderPass(&renderPassInfo, nullptr, &m_RenderPass); result != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("failed to create render pass!");
    return false;
  }
  return true;
}

//===============================================================================
bool VulkanPipeline::CreateGraphicsPipeline()
{
  auto verShaderCode  = test::ReadFile("shaders/simpleShader.vert.spv");
  auto fragShaderCode = test::ReadFile("shaders/simpleShader.frag.spv");

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
  viewport.width    = static_cast<float>(m_Extent.width);
  viewport.height   = static_cast<float>(m_Extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vk::Rect2D scissor{};
  scissor.offset = vk::Offset2D{0, 0};
  scissor.extent = m_Extent;

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

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType                  = vk::StructureType::ePipelineLayoutCreateInfo;
  pipelineLayoutInfo.setLayoutCount         = 0;       // Optional
  pipelineLayoutInfo.pSetLayouts            = nullptr; // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0;       // Optional
  pipelineLayoutInfo.pPushConstantRanges    = nullptr; // Optional

  if (m_Device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_PipelineLayout) != vk::Result::eSuccess)
  {
    LOG_CORE_ERROR("Failed to create pipeline layout");
    return false;
  }

  return true;
}

//===============================================================================
vk::ShaderModule VulkanPipeline::CreateShaderModule(const std::vector<char>& code)
{
  vk::ShaderModuleCreateInfo createInfo{};
  createInfo.sType    = vk::StructureType::eShaderModuleCreateInfo;
  createInfo.codeSize = code.size();
  createInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());
  return m_Device.createShaderModule(createInfo);
}

//===============================================================================
vk::Format VulkanPipeline::FindDepthFormat()
{
  return vk::Format::eD32Sfloat;
  // return m_VulkanContext.FindSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
  //                                            vk::ImageTiling::eOptimal,
  //                                            vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

//===============================================================================
} // namespace four
