#pragma once

#include "core/core.hpp"

#include <vulkan/vulkan.hpp>

#include "window/glfw/glfwWindow.hpp"

namespace four
{

const std::array<const char*, 1> ValidationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool EnableValidationLayers = false;
#else
constexpr bool EnableValidationLayers = true;
#endif

class Renderer
{
public:
  explicit Renderer(Window<GlfwWindow>& window);
  ~Renderer();

  Renderer(const Renderer&)                = delete;
  Renderer& operator=(const Renderer&)     = delete;
  Renderer(Renderer&&) noexcept            = delete;
  Renderer& operator=(Renderer&&) noexcept = delete;

  bool Init();
  void Shutdown();


private:
  bool InitVulkan();
  void CreateInstance();
  void SetupDebugMessenger();

  std::vector<const char*> GetRequiredExtensions();

  static void                           PrintExtensionsSupport();
  static bool                           CheckValidationLayerSupport();
  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSensivity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/);

private:
  Window<GlfwWindow>&        m_Window;
  vk::Instance               m_Instance;
  vk::DebugUtilsMessengerEXT m_DebugMessenger;
};
} // namespace four
