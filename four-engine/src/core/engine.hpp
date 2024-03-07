#pragma once

#include "core/core.hpp"

#include "core/imgui/imguiLayer.hpp"
#include "core/layerStack.hpp"

// test vulkan
#include "renderer/vulkan/vulkPipline.hpp"
#include "renderer/vulkan/vulkDevice.hpp"
#include "renderer/vulkan/vulkSwapChain.hpp"
#include "renderer/vulkan/vulkModel.hpp"

// check to use proper window
#ifdef FOUR_USE_SDL
#include "window/sdl/sdlWindow.hpp"
using UsingWindow = four::SdlWindow;

#elif FOUR_USE_GLFW
#include "window/glfw/glfwWindow.hpp"
using UsingWindow = four::GlfwWindow;
#endif // FOUR_USE_SDL

namespace four
{

class Engine
{
public:
  /**
 * @brief Deleted Default Constructor
 */
  Engine() = delete;

  /**
   * @brief Deleted move constructer
   */
  Engine(Engine&&) noexcept = delete;

  /**
   * @brief Deleted move assignment operator
   */
  Engine& operator=(Engine&&) noexcept = delete;

  /**
   * @brief Deleted copy constructer
   */
  Engine(const Engine&) = delete;

  /**
   * @brief Deleted copy assignment operator
   */
  Engine& operator=(const Engine&) = delete;

  /**
   * @brief Default destructor
   */
  ~Engine();

  /**
   * @brief Initalize Engine and its dependecies
   *
   * @param title value of string to use for title of the window
   * @param width value for width of the window
   * @param height value for height of the window
   * @return reference to created Engine ( doesn't reqired to handle lifetime )
   */
  static Engine* Init(std::string_view title, uint32_t width, uint32_t height)
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
   * @see Init(std::string_view,uint32_t,uint32_t)
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

  [[nodiscard]] inline auto* GetWindow() noexcept
  {
    return m_Window.get();
  }

  /**
   * @brief Shutdown and free resources of Engine
   */
  static void Shutdown();

private:
  /**
   * @brief Constructor of the Engine
   *
   * @param title value to use for title of window
   * @param width value to use for width of window
   * @param height value to use for height of the window
   */
  explicit Engine(std::string_view title, uint32_t width, uint32_t height);

  /**
   * @brief Initialize window for Engine
   *
   * @param title value to use for title of window
   * @param width value to use for width of window
   * @param height value to use for height of the window
   * @return true if successfuly iniliazie
   */
  [[nodiscard]] bool InitWindow(std::string_view title, uint32_t width, uint32_t height);


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
  void OnResize(uint32_t width, uint32_t height);

private:
  void LoadModels();
  void CreatePipeLineLayout();
  void CreatePipeLine();
  void CreateCommandBuffers();
  void DrawFrame();
  void ReCreateSwapChain();
  void RecordCommandBuffers(uint32_t imageIndex);

private:
  /** singletone instance of Engine */
  static std::unique_ptr<Engine> sm_Instance;

  /** imgui layer stacks for UI */
  LayerStack<ImGuiLayer> m_ImGuiLayer;

  /** main window of the Engine */
  std::unique_ptr<Window<UsingWindow>> m_Window;

  /** TODO: will be abstracted in the future */
  /** vulkan */
  std::unique_ptr<VulkDevice>    m_VulkDevice;
  std::unique_ptr<VulkSwapChain> m_SwapChain;
  std::unique_ptr<VulkPipeline>  m_VulkPipeline;
  VkPipelineLayout               m_PipelineLayout;
  std::vector<VkCommandBuffer>   m_CommandBuffers;
  std::unique_ptr<VulkModel>     m_VulkModel;
};
} // namespace four
