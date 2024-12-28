#pragma once

#include "GLFW/glfw3.h"
#include "core/core.hpp"

namespace four
{

/**
 * @brief enum to represent event type 
 * types are Window, App, Key, Mouse and base on action.
 */
enum class EventType : u8
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

enum class KeyEventValue : u8
{
  None = 0,
  KeyA = GLFW_KEY_A,
  KeyB = GLFW_KEY_B,
  KeyC = GLFW_KEY_C,
  KeyD = GLFW_KEY_D,
  KeyE = GLFW_KEY_E,
  KeyF = GLFW_KEY_F,
  KeyG = GLFW_KEY_G,
  KeyH = GLFW_KEY_H,
  KeyI = GLFW_KEY_I,
  KeyJ = GLFW_KEY_J,
  KeyK = GLFW_KEY_K,
  KeyL = GLFW_KEY_L,
  KeyM = GLFW_KEY_M,
  KeyN = GLFW_KEY_N,
  KeyO = GLFW_KEY_O,
  KeyP = GLFW_KEY_P,
  KeyQ = GLFW_KEY_Q,
  KeyR = GLFW_KEY_R,
  KeyS = GLFW_KEY_S,
  KeyT = GLFW_KEY_T,
  KeyU = GLFW_KEY_U,
  KeyV = GLFW_KEY_V,
  KeyW = GLFW_KEY_W,
  KeyX = GLFW_KEY_X,
  KeyY = GLFW_KEY_Y,
  KeyZ = GLFW_KEY_Z,
};

/**
 * @brief represent Category whitch this event belong to
 * seperating App, Input, Key, Mouse movement, Mouse buttons
 */
enum class EventCategory : u8
{
  None                     = 0U,
  EventCategoryApplication = 1U,
  EventCategoryInput       = 1U << 1U,
  EventCategoryKeyboard    = 1U << 2U,
  EventCategoryMouse       = 1U << 3U,
  EventCategoryMouseButton = 1U << 4U,
};

inline EventCategory operator&(EventCategory lhs, EventCategory rhs)
{
  return static_cast<EventCategory>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}

/**
  * @brief Event interface class
  * Parent of all interface in applicaiton
  * @param Derived derived class that is using this class as Parent
  * @param Args callback function arguments
  */
template <typename Derived, typename... Args>
class FOUR_ENGINE_API Event
{

public:
  /** call back function signiture */
  using FunctionType = std::function<void(Args...)>;
  /**
    * @brief Get event type of the event 
    * @return return Event type
    */
  [[nodiscard]] inline static EventType GetEventType()
  {
    return Derived::GetEventType();
  }

  /**
    * @brief reutrn category of event
    * @return category of event
    */
  [[nodiscard]] inline static EventCategory GetEventCategory()
  {
    return Derived::GetEventCategory();
  }

  /**
    * @brief setup call back for event
    * Set specific function callback or lambda for event trigger
    * @todo : temperory using single call back
    */
  void SetupCallBack(FunctionType callback)
  {
    m_Callback = callback;
  }

  /**
    * @brief notify callback function for changes
    * call callback function with parameters or none 
    */
  void Notify(Args... args)
  {
    if (m_Callback)
    {
      m_Callback(args...);
    }
  }

  /**
    * @brief check if belong to provided category
    * Check if the category is supported or not
    */
  bool IsInCategory(EventCategory category)
  {
    return static_cast<bool>(GetEventCategory() & category);
  }

private:
  friend Derived;

  /** temperory single call back for event */
  // @todo : consider having list of listener like observer design pattern
  FunctionType m_Callback;
};

template <typename T>
class EventDispatcher
{
public:
  explicit EventDispatcher(T&& event)
  {
    m_EventList.insert(T::GetEventType, event);
  }

  template <typename... Args>
  void Dispatch(EventType type, Args... args)
  {
    auto itEvent = m_EventList.find(type);
    if (itEvent != m_EventList.end())
    {
      itEvent.second.Notify(args...);
    }
  }

private:
  std::unordered_map<EventType, Event<T>> m_EventList;
};


} // namespace four
