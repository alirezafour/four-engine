#include "renderer/vulkan/VKHelpers.hpp"

namespace four::vkUtils
{

vk::PipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(vk::ShaderStageFlagBits stage,
                                                                vk::ShaderModule        shaderModule,
                                                                const char*             entry)
{
  return {.stage = stage, .module = shaderModule, .pName = entry};
}
} // namespace four::vkUtils
