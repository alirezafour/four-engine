#pragma once

#include "core/core.hpp"

#include "core/layer.hpp"

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
  LayerStack()                             = default;
  LayerStack(const LayerStack&)            = delete;
  LayerStack(LayerStack&&)                 = delete;
  LayerStack& operator=(const LayerStack&) = delete;
  LayerStack& operator=(LayerStack&&)      = delete;

  explicit LayerStack(std::vector<std::unique_ptr<T>> m_Layers) : m_Layers(std::move(m_Layers))
  {
    if (m_Layers.empty())
    {
      m_Layers[0]->Init();
    }
    for (auto& layer : m_Layers)
    {
      layer->OnAttach();
    }
  } /**
   * @brief accept and stop layer in layer stack
   *
   * @param layer the layer to add
   * @return raw referance to the pushed layer
   */
  T* PushLayer(std::unique_ptr<T> layer)
  {
    // call init if first layer added
    if (m_Layers.empty())
    {
      layer->Init();
    }

    LOG_CORE_INFO("Layer stack: PushLayer");
    layer->OnAttach();
    m_Layers.push_back(std::move(layer));
    return m_Layers.front().get();
  }
  ~LayerStack()
  {
    Shutdown();
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

    // check if its last layer to remove call Shutdown on layer
    if (itListToRemove.size() > 0 && itListToRemove.size() == m_Layers.size())
    {
      itListToRemove[0]->Shutdown();
    }
    m_Layers.erase(itListToRemove.begin(), itListToRemove.end());
  }

  void OnUpdate()
  {
    for (auto& layer : m_Layers)
    {
      layer->OnUpdate();
    }
  }

  void Shutdown()
  {
    if (m_Layers.size() > 0)
    {
      for (auto& layer : m_Layers)
      {
        layer->OnDetach();
      }
      // call Shutdown()
      m_Layers[0]->Shutdown();
    }
    m_Layers.clear();
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
