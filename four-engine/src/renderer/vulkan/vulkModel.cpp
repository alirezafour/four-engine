#include "four-pch.h"

#include "renderer/vulkan/vulkDevice.hpp"
#include "renderer/vulkan/vulkModel.hpp"

namespace four
{

VulkModel::VulkModel(VulkDevice& device, std::vector<Vertex>& vertices) : m_VulkDevice{device}
{
  CreateVertexBuffers(vertices);
}
VulkModel::~VulkModel()
{
  vkDestroyBuffer(m_VulkDevice.GetDevice(), m_VertexBuffer, nullptr);
  vkFreeMemory(m_VulkDevice.GetDevice(), m_VertexBufferMemory, nullptr);
}


void VulkModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
{
  m_VertexCount = static_cast<uint32_t>(vertices.size());
  assert(m_VertexCount >= 3 && "Vertex count must be at least 3");
  vk::DeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
  m_VulkDevice.CreateBuffer(bufferSize,
                            vk::BufferUsageFlagBits::eVertexBuffer,
                            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                            m_VertexBuffer,
                            m_VertexBufferMemory);

  void* data = m_VulkDevice.GetDevice().mapMemory(m_VertexBufferMemory, 0, bufferSize);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  m_VulkDevice.GetDevice().unmapMemory(m_VertexBufferMemory);
}

void VulkModel::Bind(vk::CommandBuffer commandBuffer)
{
  std::array<vk::Buffer, 1>     buffers{m_VertexBuffer};
  std::array<vk::DeviceSize, 1> offsets{0};
  commandBuffer.bindVertexBuffers(0, 1, buffers.data(), offsets.data());
}

void VulkModel::Draw(vk::CommandBuffer commandBuffer)
{
  commandBuffer.draw(m_VertexCount, 1, 0, 0);
}

std::vector<vk::VertexInputBindingDescription> VulkModel::Vertex::GetBindingDescriptions()
{
  return {{0, sizeof(Vertex), vk::VertexInputRate::eVertex}};
}
std::vector<vk::VertexInputAttributeDescription> VulkModel::Vertex::GetAttributeDescriptions()
{
  // location, binding, format, offset
  return {{0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, position)},
          {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)}};
}

} // namespace four
