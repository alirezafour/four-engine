#pragma once

#include "spdlog/fmt/bundled/core.h"
#include <functional>
#include <stdint.h>
#include <utility>
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

/**
  * @brief Event interface class
  * Parent of all interface in applicaiton
  * @param Derived derived class that is using this class as Parent
  * @param Args callback function arguments
  */
template <typename Derived, typename... Args>
class Event
{
  /** call back function signiture */
  using FunctionType = std::function<void(Args...)>;

public:
  /**
    * @brief Get event type of the event 
    * @return return Event type
    */
  EventType GetEventType() const
  {
    return static_cast<const Derived*>(this)->GetEventType();
  }

  /**
    * @brief reutrn category of event
    * @return category of event
    */
  EventCategory GetEventCategory() const
  {
    return static_cast<const Derived*>(this)->GetEventCategory();
  }

  /**
    *
    */
  void SetupCallBack(FunctionType callback)
  {
    Event<Derived>::m_Callback = callback;
  }

  void notify(Args... args)
  {
    if (m_Callback)
      m_Callback(args...);
  }

  bool IsInCategory(EventCategory category)
  {
    return GetEventCategory() & category;
  }

private:
  friend Derived;
  FunctionType m_Callback;
};


} // namespace four
