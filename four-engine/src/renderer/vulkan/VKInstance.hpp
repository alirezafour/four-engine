#pragma once

#include "core/core.hpp"

#include "vulkan/vulkan.hpp"


namespace four
{

class FOUR_ENGINE_API VKInstance
{
public:
  VKInstance(const VKInstance&)            = delete;
  VKInstance(VKInstance&&)                 = delete;
  VKInstance& operator=(const VKInstance&) = delete;
  VKInstance& operator=(VKInstance&&)      = delete;
  // explicit VKInstance(const CreateInfo& info, std::string_view engineName, std::string_view applicationName);
  ~VKInstance();
  [[nodiscard]] vk::Instance GetInstance() const
  {
    return m_Instance;
  }

private:
  vk::Instance m_Instance;
  // std::vector<VKPhysicalDevice>   m_Devices;
  // std::vector<VKDeviceProperties> m_DeviceProperties;
};
} // namespace four
