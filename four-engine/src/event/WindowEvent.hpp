#pragma once

#include "event/event.hpp"

namespace four
{

/**
  * @brief Window Close Event
  * Event that triggered when application close event happen form window
  */
class WindowCloseEvent : public Event<WindowCloseEvent>
{
public:
  /** CRTP overrited Get event type */
  [[nodiscard]] inline EventType GetEventType() const
  {
    return EventType::WindowClose;
  }

  /** CRTP overrited Get event category */
  [[nodiscard]] inline EventCategory GetEventCategory() const
  {
    return EventCategory::EventCategoryApplication;
  }
};

/**
  * @brief Window Resize event
  * Resize event triggered when window resize from window
  */
class WindowResizeEvent : public Event<WindowResizeEvent>
{
public:
  /** CRTP overrited Get event type */
  [[nodiscard]] inline EventType GetEventType() const
  {
    return EventType::WindowClose;
  }

  /** CRTP overrited Get event category */
  [[nodiscard]] inline EventCategory GetEventCategory() const
  {
    return EventCategory::EventCategoryApplication;
  }
};

} // namespace four
