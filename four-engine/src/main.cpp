
#include "SDL_events.h"
#include "window/window.hpp"
#include "window/sdl/sdlWindow.hpp"
#include <memory>

int main(int argc, char** argv)
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

  return 0;
}
