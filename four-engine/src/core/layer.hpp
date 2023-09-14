#pragma once

namespace four
{

template <class Derived>
class Layer
{
public:
  void OnAttach()
  {
    static_cast<Derived*>(this)->OnAttach();
  }
  void OnDetach()
  {
    static_cast<Derived*>(this)->OnDetach();
  }
  void OnEvent()
  {
    static_cast<Derived*>(this)->OnEvent();
  }

private:
  friend Derived;
};

} // namespace four
