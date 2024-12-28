#pragma once

#include "core/core.hpp"

#include "event/WindowEvent.hpp"

namespace four
{

struct WindowExtent
{
  uint32_t               width;
  uint32_t               height;
  [[nodiscard]] uint32_t GetWidth() const noexcept
  {
    return width;
  }
  [[nodiscard]] uint32_t GetHeight() const noexcept
  {
    return height;
  }
};

/**
  * @brinef iterface / builder class for window
  * this class is base class window and it also act as builder for window creation
  */
template <typename Derived>
class FOUR_ENGINE_API Window
{
  friend Derived;
  using PossibleEvents = std::variant<WindowCloseEvent, WindowResizeEvent>;

public:
  Window(const Window&)                = delete;
  Window(Window&&) noexcept            = default;
  Window& operator=(const Window&)     = delete;
  Window& operator=(Window&&) noexcept = default;
  virtual ~Window()                    = default;
  /**
    * @brief Create window of the requested type
    * Create Window of the type it can throw exception if fail
    * @param title the title for the window
    * @param width window width
    * @param height window height
    */
  [[nodiscard]] static Derived CreateWindow(std::string_view title, uint32_t width, uint32_t height) noexcept
  {
    static_assert(std::derived_from<Derived, Window<Derived>>, "your window class is not derived form Window<>.");
    static_assert(std::constructible_from<Derived, std::string_view, uint32_t, uint32_t>,
                  "Constrcutor with (std::string_view, uint32_t, uint32_t) parameter is not exist in derived class.");
    return Derived{title, width, height};
  }

  [[nodiscard]] auto GetHandle() const noexcept
  {
    return GetDerived()->GetHandleImpl();
  }

  /**
   * @brief get extent of the window
   * @return extent of the window
   */
  [[nodiscard]] WindowExtent GetExtent() const noexcept
  {
    return {GetWidth(), GetHeight()};
  }

  /**
   * @brief get width of the window
   * @return width of the window
   */
  [[nodiscard]] uint32_t GetWidth() const noexcept
  {
    return GetDerived()->GetWidthImpl();
  }

  /**
   * @brief get height of the window
   * @return height of the window
   */
  [[nodiscard]] uint32_t GetHeight() const noexcept
  {
    return GetDerived()->GetHeightImpl();
  }

  /**
   * @brief call on update each frame
   * it call onUpdate of derived class
   */
  void OnUpdate()
  {
    GetDerived()->OnUpdateImpl();
  }

  /**
   * @brief register key event
   * @param event key event to register
   */
  template <typename T>
  void RegisterEvent(T&& event)
  {
    GetDerived()->RegisterEventImpl(std::forward<T>(event));
  }

  /**
   * @brief bool operator for window
   * @return true if window is not null
   */
  explicit operator bool() const
  {
    return GetHandle() != nullptr;
  }

  /**
   * @brief should be call before exit
   * it call shutdown of derived class
   */
  void Shutdown()
  {
    GetDerived()->ShutdownImpl();
  }

  /**
   * @brief check if user close the window
   * @return true if user close the window
   */
  [[nodiscard]] bool ShouldClose() const noexcept
  {
    return GetDerived()->ShouldCloseImpl();
  }

  /**
   * @brief check if window was resized
   * @return true if window was resized
   */
  [[nodiscard]] bool WasWindowResized() const noexcept
  {
    return GetDerived()->WasWindowResizedImpl();
  }

  /**
   * @brief reset window resize flag
   */
  void ResetWindowResized()
  {
    GetDerived()->ResetWindowResizedImpl();
  }

  /**
   * @brief wait for events
   */
  void WaitEvents()
  {
    GetDerived()->WaitEventsImpl();
  }

  std::vector<const char*> GetVulkanRequiredExtensions()
  {
    return GetDerived()->GetVulkanRequiredExtensionsImpl();
  }


private:
  Window() = default;

  /**
   * @brief cast and return derived object pointer
   * @return non-const pointer to derived class
   */
  Derived* GetDerived()
  {
    return static_cast<Derived*>(this);
  }

  /**
   * @brief cast and return derived object pointer as const
   * @return const pointer to derived class
   */
  const Derived* GetDerived() const
  {
    return static_cast<const Derived*>(this);
  }

  std::vector<PossibleEvents> m_WindowEvents;
};
} // namespace four
