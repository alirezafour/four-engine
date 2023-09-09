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
  explicit SdlWindow(std::string_view title, std::int32_t width, int32_t height);

  /**
    * @brief shutdown the sdl and destroy window 
    */
  ~SdlWindow();

  /**
    * return row pointer ot sdl window
    * @brienf return row pointer to the sdl window
    * @return row pointer to sdl window
    */
  SDL_Window* GetWindow() const noexcept
  {
    return m_SdlWindow;
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
};
} // namespace four
