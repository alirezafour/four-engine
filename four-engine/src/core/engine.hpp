#pragma once

#include "core/core.hpp"

#include "core/imgui/imguiLayer.hpp"
#include "core/layerStack.hpp"

// renderer
#include "renderer/vulkan/vulkanRenderer.hpp"

#include "window/glfw/glfwWindow.hpp"

namespace four
{

constexpr bool FrameCapEnabled = true;
constexpr u32  TargetFPS       = 60;
constexpr f32  TargetFrameTime = 1000.0F / static_cast<f32>(TargetFPS);

class Application;

class FOUR_ENGINE_API Engine
{
public:
  Engine()                             = delete;
  Engine(Engine&&) noexcept            = delete;
  Engine& operator=(Engine&&) noexcept = delete;
  Engine(const Engine&)                = delete;
  Engine& operator=(const Engine&)     = delete;
  ~Engine();

  /**
   * @brief Initalize Engine and its dependecies
   *
   * @param title value of string to use for title of the window
   * @param width value for width of the window
   * @param height value for height of the window
   * @return reference to created Engine ( doesn't reqired to handle lifetime )
   */
  static Engine* Init(std::string_view title, u32 width, u32 height)
  {
    if (sm_Instance)
    {
      LOG_CORE_WARN("Engine Already initialized.");
      return sm_Instance.get();
    }
    sm_Instance = std::unique_ptr<Engine>(new Engine(title, width, height));
    return sm_Instance.get();
  }

  /**
   * @brief Return reference to instance of the Engine 
   * Engine shoud be initialized before using this
   * @see Init(std::string_view,u32,u32)
   * @return Reference to instace of the Engine ( doesn't required to handle lifetime )
   */
  static Engine* Get()
  {
    if (sm_Instance)
    {
      return sm_Instance.get();
    }

    LOG_CORE_ERROR("Engine is not initialized.");
    return nullptr;
  }

  /**
   * @brief Run main loop of the Engine
   */
  void Run();

  [[nodiscard]] auto* GetWindow() noexcept
  {
    return &m_Window;
  }

  /**
   * @brief Shutdown and free resources of Engine
   */
  static void Shutdown();

  void AddApplicaiton(Application* app)
  {
    m_Application = app;
  }

  void AddImGuiLayer(std::unique_ptr<ImGuiLayer> layer)
  {
    m_ImGuiLayer.PushLayer(std::move(layer));
  }

private:
  /**
   * @brief Constructor of the Engine
   *
   * @param title value to use for title of window
   * @param width value to use for width of window
   * @param height value to use for height of the window
   */
  explicit Engine(std::string_view title, u32 width, u32 height);

  /**
   * @brief Initialize window for Engine
   *
   * @param title value to use for title of window
   * @param width value to use for width of window
   * @param height value to use for height of the window
   * @return true if successfuly iniliazie
   */
  [[nodiscard]] bool InitWindow(std::string_view title, u32 width, u32 height);


  /**
   * @brief Initalize log 
   *
   * @return true if successfuly Initialize
   */
  [[nodiscard]] bool InitLog();

  /**
   * @brief Initialize ImGuiLayer 
   *
   * @return true if successfuly Initialize
   */
  [[nodiscard]] bool InitImGuiLayerStack();

  /**
   * @brief Get called on window resize 
   *
   * @param width value of new width
   * @param height value of new height
   */
  void OnResize(u32 width, u32 height);

private:
  /** singletone instance of Engine */
  static std::unique_ptr<Engine> sm_Instance;

  /** main window of the Engine */
  WindowType m_Window;

  /** renderer */
  RendererType m_Renderer;

  /** imgui layer stacks for UI */
  LayerStack<ImGuiLayer> m_ImGuiLayer;

  /** reference to app Application */
  Application* m_Application;
};
} // namespace four
