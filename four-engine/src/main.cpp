
#include "SDL_events.h"
#include "core/log.hpp"
#include "window/window.hpp"
#include "window/sdl/sdlWindow.hpp"
#include <memory>

int main(int argc, char** argv)
{
  try
  {
    auto w = four::Window<four::SdlWindow>::CreateWindow("title", 800, 600);

    bool isRunning = true;
    while (isRunning)
    {
      SDL_Event event;
      if (SDL_PollEvent(&event))
        if (event.type == SDL_EventType::SDL_EVENT_QUIT)
          isRunning = false;
    }

  } catch (std::exception& e)
  {
    LOG_CORE_WARN(e.what());
    return 1;
  }

  return 0;
}
