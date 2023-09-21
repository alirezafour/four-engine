#pragma once

namespace four
{

/**
 * @brief enum to represent event type 
 * types are Window, App, Key, Mouse and base on action.
 */
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

/**
 * @brief represent Category whitch this event belong to
 * seperating App, Input, Key, Mouse movement, Mouse buttons
 */
enum class EventCategory : uint8_t
{
  None                     = 0U,
  EventCategoryApplication = 1U,
  EventCategoryInput       = 1U << 1U,
  EventCategoryKeyboard    = 1U << 2U,
  EventCategoryMouse       = 1U << 3U,
  EventCategoryMouseButton = 1U << 4U,
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
    return GetEventCategory() & category;
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
