#pragma once

#include "event/event.hpp"

namespace four
{

class WindowCloseEvent : public Event<WindowCloseEvent>
{
public:
  [[nodiscard]] inline EventType GetEventType() const
  {
    return EventType::WindowClose;
  }

  [[nodiscard]] inline EventCategory GetEventCategory() const
  {
    return EventCategory::EventCategoryApplication;
  }
};

class WindowResizeEvent : public Event<WindowResizeEvent>
{
public:
  [[nodiscard]] inline EventType GetEventType() const
  {
    return EventType::WindowClose;
  }

  [[nodiscard]] inline EventCategory GetEventCategory() const
  {
    return EventCategory::EventCategoryApplication;
  }
};

} // namespace four
