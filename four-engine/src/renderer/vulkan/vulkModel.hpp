#pragma once

#include "renderer/vulkan/vulkDevice.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

namespace four
{

class VulkModel
{
public:
  struct Vertex
  {
    glm::vec2 position;

    static std::vector<VkVertexInputBindingDescription>   GetBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
  };

  VulkModel(VulkDevice& m_VulkDevice, std::vector<Vertex>& vertices);
  ~VulkModel();

  VulkModel(VulkModel&)                  = delete;
  VulkModel(VulkModel&&)                 = delete;
  VulkModel& operator=(const VulkModel&) = delete;
  VulkModel& operator=(VulkModel&&)      = delete;

  void Bind(VkCommandBuffer commandBuffer);
  void Draw(VkCommandBuffer commandBuffer);

private:
  void CreateVertexBuffers(const std::vector<Vertex>& vertices);

private:
  VulkDevice&    m_VulkDevice;
  VkBuffer       m_VertexBuffer;
  VkDeviceMemory m_VertexBufferMemory;
  uint32_t       m_VertexCount;
};

} //namespace four
