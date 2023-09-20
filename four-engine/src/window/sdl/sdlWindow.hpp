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
  explicit SdlWindow(std::string_view title, uint32_t width, uint32_t height);

  /**
   * @brief Deleted default constructor
   */
  SdlWindow() = delete;

  /**
   * @brief Deleted coppy constructor
   */
  SdlWindow(const SdlWindow&) = delete;

  /**
   * @brief Deleted move constructor
   */
  SdlWindow(SdlWindow&&) = delete;

  /**
   * @brief Deleted copy assignmet
   */
  SdlWindow& operator=(const SdlWindow&) = delete;

  /**
   * @brief Deleted move assignmet
   *
   * @return left side of the operator
   */
  SdlWindow& operator=(SdlWindow&&) = delete;

  /**
    * @brief Shutdown the sdl and destroy window 
    */
  ~SdlWindow();

  /**
    * return Raw pointer ot sdl window
    * @brienf return row pointer to the sdl window
    * @return row pointer to sdl window
    */
  [[nodiscard]] SDL_Window* GetWindow() const noexcept
  {
    return m_SdlWindow;
  }

  /**
   * @brief Get current width of the window 
   *
   * @return current width of window
   */
  [[nodiscard]] inline uint32_t GetWidth() const noexcept
  {
    return m_Width;
  }

  /**
   * @brief Get current height of window
   *
   * @return the current height of window
   */
  [[nodiscard]] inline uint32_t GetHeight() const noexcept
  {
    return m_Height;
  }

  /**
   * @todo : temperory replace with proper implementation ( using dispatch )
   * @brief Get existed event base on provided class
   *
   * @return object of the event
   */
  template <typename T>
  [[nodiscard]] T& GetEvent()
  {
    if constexpr (std::is_same_v<T, WindowCloseEvent> || std::is_same_v<T, WindowResizeEvent>)
    {
      if constexpr (std::is_same_v<T, WindowCloseEvent>)
      {
        return m_CloseEvent;
      }
      else
      {
        return m_ResizeEvent;
      }
    }
    else
    {
      static_assert(std::is_same_v<T, WindowResizeEvent>, "Event not exist");
      return nullptr;
    }
  }

  /**
   * @brief Get called on every frame update
   */
  void OnUpdate();

private:
  /**
    * Destroy window and shutdown subsystem video
    * @brienf destroy window and subsystem
    */
  void DestroyWindow();

  /**
   * @brief Get Called on any window event
   *
   * @param event sdl event that
   */
  void OnEvent(const SDL_Event& event);

private:
  /** the window */
  SDL_Window* m_SdlWindow{};

  /** current width of the window */
  uint32_t m_Width{};

  /** current height of the window */
  uint32_t m_Height{};

  // Events
  WindowCloseEvent  m_CloseEvent;
  WindowResizeEvent m_ResizeEvent;
};
} // namespace four
