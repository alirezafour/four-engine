#include "four-pch.h"

#include "window/sdl/sdlWindow.hpp"

#include "SDL_error.h"
#include "SDL_events.h"
#include "SDL_init.h"
#include "SDL_video.h"

#include "core/log.hpp"

namespace four
{

SdlWindow::~SdlWindow()
{
  DestroyWindow();
}

void SdlWindow::DestroyWindow()
{
  if (m_SdlWindow != nullptr)
  {
    LOG_CORE_INFO("Window Deleted.");
    SDL_DestroyWindow(m_SdlWindow);
  }

  SDL_QuitSubSystem(SDL_InitFlags::SDL_INIT_VIDEO);
}


SdlWindow::SdlWindow(std::string_view title, uint32_t width, uint32_t height) :
Window<SdlWindow>(),
m_Width(width),
m_Height(height)
{
  if (SDL_InitSubSystem(SDL_InitFlags::SDL_INIT_VIDEO) != 0)
  {
    LOG_CORE_ERROR("Error on init: {}", SDL_GetError());
    throw std::exception("failed to init sdl subsystem");
  }

  m_SdlWindow = SDL_CreateWindow(title.data(),
                                 static_cast<int>(width),
                                 static_cast<int>(height),
                                 SDL_WindowFlags::SDL_WINDOW_RESIZABLE);
  if (m_SdlWindow == nullptr)
  {
    LOG_CORE_ERROR("failed creating window: {}", SDL_GetError());
    DestroyWindow();
    throw std::exception("failed to create sdl window");
  }
  LOG_CORE_INFO("Window Created.");
}

void SdlWindow::OnUpdate()
{
  SDL_Event event;
  while (SDL_PollEvent(&event) != 0)
  {
    OnEvent(event);
  }
}

void SdlWindow::OnEvent(const SDL_Event& event)
{
  switch (event.type)
  {
    case SDL_EventType::SDL_EVENT_QUIT:
    {
      m_CloseEvent.Notify();
      break;
    }
    case SDL_EventType::SDL_EVENT_WINDOW_RESIZED:
    {
      m_ResizeEvent.Notify(0, 0);
      break;
    }
  }
}

} // namespace four
