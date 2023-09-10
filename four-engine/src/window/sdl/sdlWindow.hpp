#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#include "window/window.hpp"

struct SDL_Window;

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

private:
  /**
    * Destroy window and shutdown subsystem video
    * @brienf destroy window and subsystem
    */
  void DestroyWindow();

private:
  /** the window */
  SDL_Window* m_SdlWindow;
  int32_t     m_Width;
  int32_t     m_Height;
};
} // namespace four
