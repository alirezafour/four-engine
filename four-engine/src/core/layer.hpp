#pragma once

#include "event/event.hpp"

namespace four
{


/**
 * @brief base class for each layer to use in application
 *
 * @tparam Derived the class type that derived from this (CRTP)
 */
template <class Derived>
class Layer
{
public:
  /**
   * @brief CRTP Init of layer (it should only be called once for each type of Layer)
   */
  bool Init()
  {
    return static_cast<Derived*>(this)->Init();
  }

  /**
   * @brief CRTP On attach that get called for initialization
   */
  void OnAttach()
  {
    static_cast<Derived*>(this)->OnAttach();
  }

  /**
   * @brief CRTP On Update that called when rendering each frame
   */
  void OnUpdate()
  {
    static_cast<Derived*>(this)->OnUpdate();
  }

  /**
   * @brief CRTP on detach that get called for uninitialization
   */
  void OnDetach()
  {
    static_cast<Derived*>(this)->OnDetach();
  }

  /**
   * @brief CRTP on Event that get called when event comes for this specific layer
   * @todo: not implemented
   */
  void OnEvent()
  {
    static_cast<Derived*>(this)->OnEvent();
  }

  /**
   * @brief CRTP On Shutdown of layer (it should only be called once for each type of Layer)
   */
  void Shutdown()
  {
    static_cast<Derived*>(this)->Shutdown();
  }

private:
  friend Derived;
};

} // namespace four
