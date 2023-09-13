#pragma once

#include "spdlog/fmt/bundled/core.h"
#include <functional>
#include <stdint.h>
namespace four
{

enum class EventType : uint8_t
{
  None = 0,
  WindowClose,
  WindowResize,
  WindowFocus,
  WindowLoseFocus,
  WindowMoved,

  AppTick,
  AppUpdate,
  AppRender,

  KeyPressed,
  KeyReleased,
  KeyTyped,

  MouseButtonPressed,
  MouseButtonReleased,
  MouseMoved,
  MouseScrolled,
};

enum class EventCategory : uint8_t
{
  None                     = 0,
  EventCategoryApplication = 1,
  EventCategoryInput       = 1 << 1,
  EventCategoryKeyboard    = 1 << 2,
  EventCategoryMouse       = 1 << 3,
  EventCategoryMouseButton = 1 << 4,
};

template <typename Derived>
class Event
{
public:
  EventType GetEventType() const
  {
    return static_cast<const Derived*>(this)->GetEventType();
  }

  EventCategory GetEventCategory() const
  {
    return static_cast<const Derived*>(this)->GetEventCategory();
  }

  bool IsHandled() const
  {
    return m_IsHandled;
  }

  void SetHandled(bool value)
  {
    m_IsHandled = value;
  }

  bool IsInCategory(EventCategory category)
  {
    return GetEventCategory() & category;
  }

private:
  friend Derived;
  bool m_IsHandled = false;
};

//dispatcher
template <typename T>
class EventDispatcher
{

public:
  // void Dispatch(T type, )
};


} // namespace four
