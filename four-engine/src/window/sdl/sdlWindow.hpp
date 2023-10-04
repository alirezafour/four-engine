#pragma once

#include "core/core.hpp"

#include "window/window.hpp"

struct SDL_Window;
union SDL_Event;

namespace four
{

class SdlWindow final : public Window<SdlWindow>
{
public:
  using WindowEventVariant = std::variant<Event<WindowCloseEvent>, Event<WindowResizeEvent, uint32_t, uint32_t>>;

  /**
    * @brief Creating a window 
    * if it fail it thorw exception
    * @param title window title
    * @param width width of the window
    * @param height height of the window
    */
  explicit SdlWindow(std::string_view title, uint32_t width, uint32_t height);

  /**
   * @brief Deleted coppy constructor
   */
  SdlWindow(const SdlWindow&) = delete;

  /**
   * @brief Deleted move constructor
   */
  SdlWindow(SdlWindow&&) = default;

  /**
   * @brief Deleted copy assignmet
   */
  SdlWindow& operator=(const SdlWindow&) = delete;

  /**
   * @brief Deleted move assignmet
   *
   * @return left side of the operator
   */
  SdlWindow& operator=(SdlWindow&&) = default;

  /**
    * @brief Shutdown the sdl and destroy window 
    */
  ~SdlWindow() final;

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
  [[nodiscard]] uint32_t GetWidth() const noexcept;

  /**
   * @brief Get current height of window
   *
   * @return the current height of window
   */
  [[nodiscard]] uint32_t GetHeight() const noexcept;

  template <typename F, typename T, typename... Args>
  void SetEventCallBack(Event<T, Args...> event, F lambda)
  {
    event.SetupCallBack(lambda);
    m_EventList[T::GetEventType()] = std::move(event);
  }

  /**
   * @brief Get called on every frame update
   */
  void OnUpdate();

  /**
   * @brief Get Called on any window event
   *
   * @param event sdl event that
   */
  void OnEvent(const SDL_Event& event);

  /**
   * @brief Shutdown window and sdl video subsystem
   */
  void Shutdown();

  /**
   * @brief Check if user closed window
   *
   * @return if user closed window and engine should stop
   */
  [[nodiscard]] bool ShouldClose() const noexcept;

private:
  /**
    * Destroy window and shutdown subsystem video
    * @brienf destroy window and subsystem
    */
  void DestroyWindow();

  /**
   * @brief GetCalled when user close window
   */
  void OnCloseEvent();

  /**
   * @brief Translate SDL Event to four event system
   *
   * @param event SDL event
   * @return equivilant event from four event 
   */
  static EventType TransformEvent(const SDL_Event& event);

  /** the window */
  SDL_Window* m_SdlWindow{};

  /** current width of the window */
  uint32_t m_Width{};

  /** current height of the window */
  uint32_t m_Height{};

  // Events
  std::unordered_map<EventType, WindowEventVariant> m_EventList;

  /** flag to know if user close the window */
  bool m_ShouldCose = false;
};
} // namespace four

/**
  * @class SdlWindow
  * @brief class to create SDL window with SDL3
  */
