#pragma once

#include "event/WindowEvent.hpp"
#include "window/window.hpp"

struct SDL_Window;
union SDL_Event;

namespace four
{

/**
  * Class to Create window with default flags with SDL3 library
  * @brief class to create SDL window with SDL3
  */
class SdlWindow : public Window<SdlWindow>
{
public:
  /**
    * @brief Creating a window 
    * if it fail it thorw exception
    * @param title window title
    * @param width width of the window
    * @param height height of the window
    */
  SdlWindow()
  {
  }
  SdlWindow(std::string_view title, std::int32_t width, int32_t height);

  /**
    * @brief shutdown the sdl and destroy window 
    */
  ~SdlWindow();

  /**
    * return row pointer ot sdl window
    * @brienf return row pointer to the sdl window
    * @return row pointer to sdl window
    */
  [[nodiscard]] SDL_Window* GetWindow() const noexcept
  {
    return m_SdlWindow;
  }

  [[nodiscard]] inline int32_t GetWidth() const noexcept
  {
    return m_Width;
  }

  [[nodiscard]] inline int32_t GetHeight() const noexcept
  {
    return m_Height;
  }

  template <typename T>
  [[nodiscard]] T& GetEvent()
  {
    if constexpr (std::is_same_v<T, WindowCloseEvent> || std::is_same_v<T, WindowResizeEvent>)
    {
      if constexpr (std::is_same_v<T, WindowCloseEvent>)
        return m_CloseEvent;
      else
        return m_ResizeEvent;
    }
    else
    {
      static_assert(std::is_same_v<T, WindowResizeEvent>, "Event not exist");
      return nullptr;
    }
  }

  void OnUpdate();

private:
  /**
    * Destroy window and shutdown subsystem video
    * @brienf destroy window and subsystem
    */
  void DestroyWindow();

  void OnEvent(const SDL_Event& event);

private:
  /** the window */
  SDL_Window* m_SdlWindow;
  int32_t     m_Width;
  int32_t     m_Height;

  // Events
  WindowCloseEvent  m_CloseEvent;
  WindowResizeEvent m_ResizeEvent;
};
} // namespace four
