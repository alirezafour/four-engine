#include "SDL_events.h"
#include "catch2/catch_test_macros.hpp"

#include "event/WindowEvent.hpp"
#include "event/event.hpp"
#include "window/window.hpp"
#include "window/sdl/sdlWindow.hpp"
#include <memory>


TEST_CASE("Constrcut SDL Window")
{
  SECTION("Using Interface")
  {

    std::unique_ptr<four::Window<four::SdlWindow>> window = std::make_unique<four::SdlWindow>("title", 200, 300);

    REQUIRE(window != nullptr);
    REQUIRE(window->GetWidth() == 200);
    REQUIRE(window->GetHeight() == 300);

    SDL_Window* sdlWindow = window->GetWindow();
    REQUIRE(sdlWindow != nullptr);
  }

  SECTION("Directing Create SDL Window")
  {

    std::unique_ptr<four::SdlWindow> window = four::Window<four::SdlWindow>::CreateWindow("title", 2, 1);
    REQUIRE(window != nullptr);
    REQUIRE(window->GetWidth() == 2);
    REQUIRE(window->GetHeight() == 1);

    SDL_Window* sdlWindow = window->GetWindow();
    REQUIRE(sdlWindow != nullptr);
  }

  SECTION("sdlWindow Events")
  {
    SECTION("Close Event")
    {
      std::unique_ptr<four::SdlWindow> window = four::Window<four::SdlWindow>::CreateWindow("title", 2, 1);

      bool eventCalled = false;

      window->SetEventCallBack(four::WindowCloseEvent(), [&eventCalled]() { eventCalled = true; });
      SDL_Event event;
      event.type = SDL_EventType::SDL_EVENT_QUIT;
      window->OnEvent(event);
      REQUIRE(eventCalled == true);
    }
    SECTION("Resize Event")
    {
      std::unique_ptr<four::SdlWindow> window = four::Window<four::SdlWindow>::CreateWindow("title", 2, 1);

      bool eventCalled = false;

      window->SetEventCallBack(four::WindowResizeEvent(),
                               [&eventCalled]([[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height)
                               { eventCalled = true; });
      SDL_Event event;
      event.type = SDL_EventType::SDL_EVENT_WINDOW_RESIZED;
      window->OnEvent(event);
      REQUIRE(eventCalled == true);
    }
  }
}

TEST_CASE("Layer Test")
{
  SECTION("Pushing")
  {
  }
}
