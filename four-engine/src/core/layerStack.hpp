#pragma once

#include "core/log.hpp"

namespace four
{

template <typename T>
class LayerStack
{
public:
  T* PushLayer(std::unique_ptr<T> layer)
  {
    LOG_CORE_INFO("Layer stack: PushLayer");
    m_Layers.push_back(std::move(layer));
    return m_Layers.front().get();
  }

  void RemoveLayer(T* layer)
  {
    if (std::erase_if(m_Layers, [layer](const auto& inLayer) { return inLayer.get() == layer; }))
    {
      LOG_CORE_INFO("LayerStack Erased element.");
    }
  }

  [[nodiscard]] std::size_t Count() const
  {
    return m_Layers.size();
  }

private:
  std::vector<std::unique_ptr<T>> m_Layers;
};

} // namespace four
