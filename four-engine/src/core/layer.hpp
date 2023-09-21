#pragma once

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
   */
  void OnEvent()
  {
    static_cast<Derived*>(this)->OnEvent();
  }

private:
  friend Derived;
};

// helpers
template <typename T>
struct IsLayer
{
  constexpr static bool value = false;
};

template <typename T>
struct IsLayer<Layer<T>>
{
  constexpr static bool value = true;
};

template <typename T>
constexpr bool IsLayer_v = IsLayer<T>::value;

} // namespace four
