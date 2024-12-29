#pragma once

#include <vulkan/vulkan.hpp>

namespace four::vkUtils
{
vk::PipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(vk::ShaderStageFlagBits stage,
                                                                vk::ShaderModule        shaderModule,
                                                                const char*             entry = "main");

}
