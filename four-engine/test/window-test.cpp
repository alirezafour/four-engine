#include "SDL_events.h"
#include "catch2/catch_test_macros.hpp"

#include "event/WindowEvent.hpp"
#include "event/event.hpp"
#include "window/window.hpp"
#include <memory>

// Check what window is used and test base on that
#ifdef FOUR_USE_SDL
#include "window/sdl/sdlWindow.hpp"
using UsedWindow = four::SdlWindow;
#endif // FOUR_USE_SDL


TEST_CASE("Constrcut Window")
{
  SECTION("Using Interface")
  {
    std::unique_ptr<four::Window<UsedWindow>> window = std::make_unique<UsedWindow>("title", 200, 300);

    REQUIRE(window != nullptr);
    REQUIRE(window->GetWidth() == 200);
    REQUIRE(window->GetHeight() == 300);

    SDL_Window* sdlWindow = window->GetWindow();
    REQUIRE(sdlWindow != nullptr);
  }

  SECTION("Directing Create Window")
  {

    std::unique_ptr<UsedWindow> window = four::Window<UsedWindow>::CreateWindow("title", 2, 1);
    REQUIRE(window != nullptr);
    REQUIRE(window->GetWidth() == 2);
    REQUIRE(window->GetHeight() == 1);

#ifdef FOUR_USE_SDL
    SDL_Window* sdlWindow = window->GetWindow();
    REQUIRE(sdlWindow != nullptr);
#endif // FOUR_USE_SDL
  }

  SECTION("Window Events")
  {
    SECTION("Close Event")
    {
      std::unique_ptr<UsedWindow> window = four::Window<UsedWindow>::CreateWindow("title", 2, 1);

      bool eventCalled = false;

      window->SetEventCallBack(four::WindowCloseEvent(), [&eventCalled]() { eventCalled = true; });
      SDL_Event event;
      event.type = SDL_EventType::SDL_EVENT_QUIT;
      window->OnEvent(event);
      REQUIRE(eventCalled == true);
    }
    SECTION("Resize Event")
    {
      std::unique_ptr<UsedWindow> window = four::Window<UsedWindow>::CreateWindow("title", 2, 1);

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
