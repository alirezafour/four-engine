#include "catch2/catch_test_macros.hpp"

#include "core/log.hpp"
#include "window/window.hpp"
#include "window/sdl/sdlWindow.hpp"
#include <memory>

TEST_CASE("Testing Window")
{
  std::unique_ptr<four::SdlWindow> window = std::make_unique<four::SdlWindow>("title", 1, 1);
  REQUIRE(window != nullptr);

  std::unique_ptr<four::SdlWindow> builder = four::Window<four::SdlWindow>::CreateWindow("title", 1, 1);
  REQUIRE(builder != nullptr);
}
