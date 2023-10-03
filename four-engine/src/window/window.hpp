#pragma once

#include "core/core.hpp"

#include "event/WindowEvent.hpp"

namespace four
{

/**
  * @brinef iterface / builder class for window
  * this class is base class window and it also act as builder for window creation
  */
template <typename Derived>
class Window
{
  using PossibleEvents = std::variant<WindowCloseEvent, WindowResizeEvent>;

public:
  virtual ~Window() = default;
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

    try
    {
      auto window = std::make_unique<Derived>(title, width, height);
      return std::move(window);
    } catch (std::exception& e)
    {
      LOG_CORE_ERROR(e.what());
    }
    return nullptr;
  }

  [[nodiscard]] auto GetWindow() const noexcept
  {
    static_assert(std::derived_from<Derived, Window<Derived>>, "your window class is not derived form Window<>.");

    return static_cast<const Derived*>(this)->GetWindow();
  }

  [[nodiscard]] inline uint32_t GetWidth() const noexcept
  {
    return static_cast<const Derived*>(this)->GetWidth();
  }

  [[nodiscard]] inline uint32_t GetHeight() const noexcept
  {
    return static_cast<const Derived*>(this)->GetHeight();
  }

  void OnUpdate()
  {
    static_cast<const Derived*>(this)->OnUpdate();
  }

  explicit operator bool() const
  {
    return GetWindow() != nullptr;
  }

  void Shutdown()
  {
    static_cast<Derived*>(this)->Shutdown();
  }


private:
  friend Derived;
  std::vector<PossibleEvents> m_WindowEvents;
};

} // namespace four
