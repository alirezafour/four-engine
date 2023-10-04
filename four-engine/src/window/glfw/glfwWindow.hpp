#include "core/core.hpp"

#include "event/WindowEvent.hpp"
#include "event/event.hpp"
#include "window/window.hpp"

struct GLFWwindow;

namespace four
{

class GlfwWindow : public Window<GlfwWindow>
{
public:
  using WindowEventVariant = std::variant<Event<WindowCloseEvent>, Event<WindowResizeEvent, uint32_t, uint32_t>>;
  explicit GlfwWindow(std::string_view title, uint32_t width, uint32_t height);
  ~GlfwWindow() override;
  GlfwWindow(GlfwWindow&&)                 = default;
  GlfwWindow& operator=(const GlfwWindow&) = delete;
  GlfwWindow& operator=(GlfwWindow&&)      = default;
  GlfwWindow(const GlfwWindow&)            = delete;

  [[nodiscard]] auto* GetWindow() const noexcept
  {
    return m_Window;
  }

  [[nodiscard]] uint32_t GetWidth() const noexcept;
  [[nodiscard]] uint32_t GetHeight() const noexcept;

  void OnUpdate();

  void Shutdown();

  [[nodiscard]] bool ShouldClose() const noexcept;

protected:
  static void GlfwErrorsCallback(int32_t /*error*/, const char* descripton)
  {
    LOG_CORE_ERROR("GLFW Error: {}", descripton);
  }

private:

private:
  GLFWwindow* m_Window;
  uint32_t    m_Width;
  uint32_t    m_Height;
};

} // namespace four
