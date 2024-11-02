#include "catch2/catch_test_macros.hpp"

#include "event/WindowEvent.hpp"
#include "window/window.hpp"
#include "core/log.hpp"
#include "renderer/renderer.hpp"

#include <memory>

#include "window/glfw/glfwWindow.hpp"
using UsedWindow = four::GlfwWindow;


TEST_CASE("Constrcut Window")
{
  // log used in every systems so it required to initialized first
  four::Log::Init();

  SECTION("Using Interface")
  {
    std::unique_ptr<four::Window<UsedWindow>> window = std::make_unique<UsedWindow>("title", 200, 300);

    REQUIRE(window != nullptr);
    REQUIRE(window->GetWidth() == 200);
    REQUIRE(window->GetHeight() == 300);

    auto* sdlWindow = window->GetHandle();
    REQUIRE(sdlWindow != nullptr);
  }

  SECTION("Directing Create Window")
  {

    std::unique_ptr<UsedWindow> window = four::Window<UsedWindow>::CreateWindow("title", 2, 1);
    REQUIRE(window != nullptr);
    REQUIRE(window->GetWidth() == 2);
    REQUIRE(window->GetHeight() == 1);
  }

  // SECTION("Window Events")
  // {
  //   SECTION("Close Event")
  //   {
  //     std::unique_ptr<UsedWindow> window = four::Window<UsedWindow>::CreateWindow("title", 2, 1);
  //
  //     bool eventCalled = false;
  //
  //     window->SetEventCallBack(four::WindowCloseEvent(), [&eventCalled]() { eventCalled = true; });
  //     SDL_Event event;
  //     event.type = SDL_EventType::SDL_EVENT_QUIT;
  //     window->OnEvent(event);
  //     REQUIRE(eventCalled == true);
  //   }
  //   SECTION("Resize Event")
  //   {
  //     std::unique_ptr<UsedWindow> window = four::Window<UsedWindow>::CreateWindow("title", 2, 1);
  //
  //     bool eventCalled = false;
  //
  //     window->SetEventCallBack(four::WindowResizeEvent(),
  //                              [&eventCalled]([[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height)
  //                              { eventCalled = true; });
  //     SDL_Event event;
  //     event.type = SDL_EventType::SDL_EVENT_WINDOW_RESIZED;
  //     window->OnEvent(event);
  //     REQUIRE(eventCalled == true);
  //   }
  // }
}
TEST_CASE("Renderer")
{
  // SECTION("Vulkan Context")
  // {
  //   std::unique_ptr<four::Window<UsedWindow>> window = std::make_unique<UsedWindow>("title", 200, 300);
  //   four::VulkanContext                       vKContext{*window};
  //   REQUIRE(vKContext.GetVulkanInstance());
  //   REQUIRE(vKContext.GetDevice());
  //   REQUIRE(vKContext.GetPhysicalDevice());
  //   REQUIRE(vKContext.GetSurface());
  //   window.reset();
  // }
  // SECTION("Vulkan Renderer")
  // {
  //   std::unique_ptr<four::Window<UsedWindow>> window = std::make_unique<UsedWindow>("title", 200, 300);
  //
  //   four::VulkanContext  vKContext{*window};
  //   four::VulkanRenderer vKRender{*window, vKContext};
  //   window.reset();
  // }
  // SECTION("Vulkan Pipeline")
  // {
  //   std::unique_ptr<four::Window<UsedWindow>> window = std::make_unique<UsedWindow>("title", 200, 300);
  //
  //   four::VulkanContext  vKContext{*window};
  //   four::VulkanRenderer vKRender{*window, vKContext};
  //   four::VulkanPipeline vKPipeline{vKContext.GetDevice(), vKContext.GetExtent(), vKRender.GetSwapChainImageFormat()};
  //   window.reset();
  // }
  // SECTION("Vulkan Renderer")
  // {
  //   std::unique_ptr<four::Window<UsedWindow>> window = std::make_unique<UsedWindow>("title", 200, 300);
  //   four::VulkanContext                       vKContext{*window};
  //   four::VulkanRenderer                      vKRender{*window, vKContext};
  //
  //   window.reset();
  // }
  // SECTION("Renderer")
  // {
  //   std::unique_ptr<four::Window<UsedWindow>> window   = std::make_unique<UsedWindow>("title", 200, 300);
  //   std::unique_ptr<four::Renderer>           renderer = std::make_unique<four::Renderer>(*window);
  //   // REQUIRE(renderer->Init());
  //   // renderer.reset();
  //   window.reset();
  // }
}
