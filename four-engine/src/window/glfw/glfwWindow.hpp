#pragma once

#include "core/core.hpp"

#include "event/windowEvent.hpp"
#include "window/window.hpp"

struct GLFWwindow;

namespace four
{

class FOUR_ENGINE_API GlfwWindow : public Window<GlfwWindow>
{
  friend class Window<GlfwWindow>;

public:
  using WindowEventVariant = std::variant<Event<WindowCloseEvent>, Event<WindowResizeEvent, uint32_t, uint32_t>>;

  /**
   * @brief Construct window
   *
   * @param title text to show as window title 
   * @param width size of width for window
   * @param height size of height for window
   */
  explicit GlfwWindow(std::string_view title, uint32_t width, uint32_t height);

  /**
   * @brief Destrcutor of window
   * if window is not properly shutdowned it re-call Shutdown()
   * @see Shutdown()
   */
  ~GlfwWindow() override;

  /**
   * @brief Deleted copy Constructor
   */
  GlfwWindow(const GlfwWindow&) = delete;

  /**
   * @brief Default Move Constructor
   */
  GlfwWindow(GlfwWindow&&) = default;

  /**
   * @brief Deleted copy assignment operator
   */
  GlfwWindow& operator=(const GlfwWindow&) = delete;

  /**
   * @brief Default move assignment operator
   */
  GlfwWindow& operator=(GlfwWindow&&) = default;

private:
  /**
   * @brief Get reference to glfw window
   *
   * @return Reference to glfw window
   */
  [[nodiscard]] auto* GetWindowImpl() const noexcept
  {
    return m_Window;
  }

  /**
   * @brief Get Width of the window
   *
   * @return width of the window 
   */
  [[nodiscard]] uint32_t GetWidthImpl() const noexcept;

  /**
   * @brief Get Height of the window
   *
   * @return Height of the window
   */
  [[nodiscard]] uint32_t GetHeightImpl() const noexcept;

  /**
   * @brief Get called on update of the engine
   * it handle events
   */
  void OnUpdateImpl();

  /**
   * @brief Called to destruct glfw window and system
   */
  void ShutdownImpl();

  /**
   * @brief Return if window need to be closed
   * when user close the application this flag is set to true
   *
   * @return flag to know that user close the application or not
   */
  [[nodiscard]] bool ShouldCloseImpl() const noexcept;

  /**
   * @brief Callback for glfw errors
   *
   * @param descripton Error descripton
   */
  static void GlfwErrorsCallback(int32_t /*error*/, const char* descripton);


private:
  /** Reference to glfw window */
  GLFWwindow* m_Window;
  uint32_t    m_Width;
  uint32_t    m_Height;
};

} // namespace four

/**
 * @class GlfwWindow
 * @brief Class to create and handle window using glfw window library
 * This class intended to be used from Window<> ( his parrent )
 */
