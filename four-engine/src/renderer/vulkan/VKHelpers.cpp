#include "renderer/vulkan/VKHelpers.hpp"

#include <fstream>

namespace four::vkUtils
{

static std::vector<char> ReadFile(const std::string& filename)
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

vk::ShaderModule CreateShaderModule(std::filesystem::path path, vk::Device device)
{
  auto code = ReadFile(path);
  return device.createShaderModule({.codeSize = code.size(), .pCode = reinterpret_cast<const uint32_t*>(code.data())});
}

vk::PipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(vk::ShaderStageFlagBits stage,
                                                                vk::ShaderModule        shaderModule,
                                                                const char*             entry)
{
  return {.stage = stage, .module = shaderModule, .pName = entry};
}
} // namespace four::vkUtils
