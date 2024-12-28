#pragma once
#include "core/core.hpp"

#include "event/event.hpp"

namespace four
{

/**
  * @brief Window Close Event
  * Event that triggered when application close event happen form window
  */
class FOUR_ENGINE_API WindowCloseEvent : public Event<WindowCloseEvent>
{
public:
  /** CRTP overrited Get event type */
  [[nodiscard]] inline static EventType GetEventType()
  {
    return EventType::WindowClose;
  }

  /** CRTP overrited Get event category */
  [[nodiscard]] inline static EventCategory GetEventCategory()
  {
    return EventCategory::EventCategoryApplication;
  }
};

/**
  * @brief Window Resize event
  * Resize event triggered when window resize from window
  */
class WindowResizeEvent : public Event<WindowResizeEvent, std::uint32_t, std::uint32_t>
{
public:
  /** CRTP overrited Get event type */
  [[nodiscard]] inline static EventType GetEventType()
  {
    return EventType::WindowResize;
  }

  /** CRTP overrited Get event category */
  [[nodiscard]] inline static EventCategory GetEventCategory()
  {
    return EventCategory::EventCategoryApplication;
  }
};

class KeyPressedEvent : public Event<KeyPressedEvent, std::uint32_t>
{
public:
  /** CRTP overrited Get event type */
  [[nodiscard]] inline static EventType GetEventType()
  {
    return EventType::KeyPressed;
  }

  /** CRTP overrited Get event category */
  [[nodiscard]] inline static EventCategory GetEventCategory()
  {
    return EventCategory::EventCategoryKeyboard;
  }
};

class MouseMovement : public Event<MouseMovement, float, float>
{
public:
  /** CRTP overrited Get event type */
  [[nodiscard]] inline static EventType GetEventType()
  {
    return EventType::MouseMoved;
  }

  /** CRTP overrited Get event category */
  [[nodiscard]] inline static EventCategory GetEventCategory()
  {
    return EventCategory::EventCategoryMouse;
  }
};
} // namespace four
