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
  VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
  m_VulkDevice.CreateBuffer(bufferSize,
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            m_VertexBuffer,
                            m_VertexBufferMemory);

  void* data = nullptr;
  vkMapMemory(m_VulkDevice.GetDevice(), m_VertexBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(m_VulkDevice.GetDevice(), m_VertexBufferMemory);
}

void VulkModel::Bind(VkCommandBuffer commandBuffer)
{
  std::array<VkBuffer, 1>     buffers{m_VertexBuffer};
  std::array<VkDeviceSize, 1> offsets{0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers.data(), offsets.data());
}

void VulkModel::Draw(VkCommandBuffer commandBuffer)
{
  vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
}

std::vector<VkVertexInputBindingDescription> VulkModel::Vertex::GetBindingDescriptions()
{
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
  bindingDescriptions[0].binding   = 0;
  bindingDescriptions[0].stride    = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return bindingDescriptions;
}
std::vector<VkVertexInputAttributeDescription> VulkModel::Vertex::GetAttributeDescriptions()
{
  // location, binding, format, offset
  return {{0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, position)},
          {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)}};
}

} // namespace four
