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
    * @brief setup call back for event
    * Set specific function callback or lambda for event trigger
    * @todo : temperory using single call back
    */
  void SetupCallBack(FunctionType callback)
  {
    Event<Derived>::m_Callback = callback;
  }

  /**
    * @brief notify callback function for changes
    * call callback function with parameters or none 
    */
  void notify(Args... args)
  {
    if (m_Callback)
      m_Callback(args...);
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


} // namespace four
