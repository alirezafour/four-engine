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
   * @brief CRTP On attach that get called for initialization
   */
  void OnAttach()
  {
    static_cast<Derived*>(this)->OnAttach();
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

private:
  friend Derived;
};

} // namespace four
