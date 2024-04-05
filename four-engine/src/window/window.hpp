#pragma once

#include "core/core.hpp"

#include "event/windowEvent.hpp"

// forward declaration
namespace vk
{
class Instance;
class SurfaceKHR;
} // namespace vk

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
  [[nodiscard]] static std::unique_ptr<Derived> CreateWindow(std::string_view title, uint32_t width, uint32_t height) noexcept
  {
    static_assert(std::derived_from<Derived, Window<Derived>>, "your window class is not derived form Window<>.");
    static_assert(std::constructible_from<Derived, std::string_view, uint32_t, uint32_t>,
                  "Constrcutor with (std::string_view, uint32_t, uint32_t) parameter is not exist in derived class.");

    auto window = std::make_unique<Derived>(title, width, height);
    FOUR_ASSERT(window != nullptr);
    return std::move(window);
  }

  [[nodiscard]] auto GetHandle() const noexcept
  {
    return GetConstDerived()->GetHandleImpl();
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
  [[nodiscard]] inline uint32_t GetWidth() const noexcept
  {
    return GetConstDerived()->GetWidthImpl();
  }

  /**
   * @brief get height of the window
   * @return height of the window
   */
  [[nodiscard]] inline uint32_t GetHeight() const noexcept
  {
    return GetConstDerived()->GetHeightImpl();
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
    return GetConstDerived()->ShouldCloseImpl();
  }

  /**
   * @brief check if window was resized
   * @return true if window was resized
   */
  [[nodiscard]] bool WasWindowResized() const noexcept
  {
    return GetConstDerived()->WasWindowResizedImpl();
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
    return GetConstDerived()->GetVulkanRequiredExtensionsImpl();
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
  const Derived* GetConstDerived() const
  {
    return static_cast<const Derived*>(this);
  }

  friend Derived;
  std::vector<PossibleEvents> m_WindowEvents;
};

} // namespace four
