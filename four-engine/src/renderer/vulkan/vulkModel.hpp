#pragma once

#include "vulkan/vulkan.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

namespace four
{
class VulkDevice;

class VulkModel
{
public:
  struct Vertex
  {
    glm::vec2 position;
    glm::vec3 color;

    static std::vector<vk::VertexInputBindingDescription>   GetBindingDescriptions();
    static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions();
  };

  VulkModel(VulkDevice& m_VulkDevice, std::vector<Vertex>& vertices);
  ~VulkModel();

  VulkModel(VulkModel&)                  = delete;
  VulkModel(VulkModel&&)                 = delete;
  VulkModel& operator=(const VulkModel&) = delete;
  VulkModel& operator=(VulkModel&&)      = delete;

  void Bind(vk::CommandBuffer commandBuffer);
  void Draw(vk::CommandBuffer commandBuffer);

private:
  void CreateVertexBuffers(const std::vector<Vertex>& vertices);

private:
  VulkDevice&      m_VulkDevice;
  vk::Buffer       m_VertexBuffer;
  vk::DeviceMemory m_VertexBufferMemory;
  uint32_t         m_VertexCount;
};

} //namespace four
