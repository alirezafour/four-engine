// #include "SDL_error.h"
// #include "SDL_events.h"
// #include "SDL_init.h"
// #include "SDL_video.h"

#include "core.hpp"


int main(int argc, char** argv)
{
  LOG_CORE_INFO("print{}", " this");

  // if (SDL_InitSubSystem(SDL_InitFlags::SDL_INIT_VIDEO))
  // {
  //     std::cout << std::format("Error on init: {}\n", SDL_GetError());
  //     return 1;
  // }
  //
  // auto window = SDL_CreateWindow("Testing SDL3", 800, 600, SDL_WindowFlags::SDL_WINDOW_RESIZABLE);
  // if (!window)
  // {
  //     std::cout << std::format("failed creating window: {}\n", SDL_GetError());
  //     SDL_QuitSubSystem(SDL_InitFlags::SDL_INIT_VIDEO);
  //     return 2;
  // }
  //
  // bool isRunning = true;
  //
  // while (isRunning)
  // {
  //     SDL_Event event;
  //     if (SDL_PollEvent(&event))
  //         if (event.type == SDL_EventType::SDL_EVENT_QUIT)
  //             isRunning = false;
  // }
  //
  // SDL_DestroyWindow(window);
  // SDL_QuitSubSystem(SDL_InitFlags::SDL_INIT_VIDEO);

  return 0;
}
