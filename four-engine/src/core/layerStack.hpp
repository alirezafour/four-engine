#pragma once

#include "core/layer.hpp"
#include <xutility>

namespace four
{

/**
 * @brief Layer Stack to store and manage life time of specific type of
 *
 * @tparam T Type of layer to store
 */
template <typename T>
  requires std::derived_from<T, Layer<T>>
class LayerStack
{
public:
  /**
   * @brief accept and stop layer in layer stack
   *
   * @param layer the layer to add
   * @return raw referance to the pushed layer
   */
  T* PushLayer(std::unique_ptr<T> layer)
  {
    LOG_CORE_INFO("Layer stack: PushLayer");
    layer->OnAttach();
    m_Layers.push_back(std::move(layer));
    return m_Layers.front().get();
  }
  ~LayerStack()
  {
    for (const auto& layer : m_Layers)
    {
      layer->OnDetach();
    }
  }

  /**
   * @brief remove layer form layer stack
   *
   * @param layer referance to layer to be removed
   */
  void RemoveLayer(T* layer)
  {
    auto itListToRemove = std::ranges::remove_if(m_Layers,
                                                 [layer](const auto& inLayer) { return inLayer.get() == layer; });
    for (auto& each : itListToRemove)
    {
      each->OnDetach();
    }
    m_Layers.erase(itListToRemove.begin(), itListToRemove.end());
  }

  /**
   * @brief return amount of layer exist in this layer stack
   *
   * @return the amount of layer in stack
   */
  [[nodiscard]] std::size_t Count() const
  {
    return m_Layers.size();
  }

private:
  /** list of layer */
  std::vector<std::unique_ptr<T>> m_Layers;
};

} // namespace four
