#include "catch2/catch_test_macros.hpp"

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

    SDL_Window* w = window->GetWindow();
    REQUIRE(w != nullptr);
  }

  SECTION("Directing Create SDL Window")
  {

    std::unique_ptr<four::SdlWindow> window = four::Window<four::SdlWindow>::CreateWindow("title", 2, 1);
    REQUIRE(window != nullptr);
    REQUIRE(window->GetWidth() == 2);
    REQUIRE(window->GetHeight() == 1);

    SDL_Window* w = window->GetWindow();
    REQUIRE(w != nullptr);
  }
}

TEST_CASE("Layer Test")
{
  SECTION("Pushing")
  {
  }
}
