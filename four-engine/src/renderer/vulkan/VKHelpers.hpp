#pragma once

#include <vulkan/vulkan.hpp>

namespace four::vkUtils
{
//===============================================================================
static std::vector<char> ReadFile(const std::string& filename);

// TODO: use expected return when error type made
vk::ShaderModule CreateShaderModule(std::filesystem::path path, vk::Device device);

vk::PipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(vk::ShaderStageFlagBits stage,
                                                                vk::ShaderModule        shaderModule,
                                                                const char*             entry = "main");

} // namespace four::vkUtils
