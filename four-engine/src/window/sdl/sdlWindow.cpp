#include "four-pch.h"

#include "window/sdl/sdlWindow.hpp"
#include "event/WindowEvent.hpp"

#include "SDL_error.h"
#include "SDL_events.h"
#include "SDL_init.h"
#include "SDL_video.h"

namespace four
{

SdlWindow::~SdlWindow()
{
  DestroyWindow();
}

uint32_t SdlWindow::GetWidth() const noexcept
{
  int width = 0;
  SDL_GetWindowSize(m_SdlWindow, &width, nullptr);
  return static_cast<uint32_t>(width);
}
uint32_t SdlWindow::GetHeight() const noexcept
{
  int height = 0;
  SDL_GetWindowSize(m_SdlWindow, nullptr, &height);
  return static_cast<uint32_t>(height);
}

void SdlWindow::Shutdown()
{
  DestroyWindow();
}

void SdlWindow::DestroyWindow()
{
  if (m_SdlWindow != nullptr)
  {
    LOG_CORE_INFO("Window Deleted.");
    SDL_DestroyWindow(m_SdlWindow);
    m_SdlWindow = nullptr;
    SDL_QuitSubSystem(SDL_InitFlags::SDL_INIT_VIDEO);
  }
}


SdlWindow::SdlWindow(std::string_view title, uint32_t width, uint32_t height) : m_Width(width), m_Height(height)
{
  if (SDL_InitSubSystem(SDL_InitFlags::SDL_INIT_VIDEO) != 0)
  {
    LOG_CORE_ERROR("Error on init: {}", SDL_GetError());
    FOUR_ASSERT(false);
    return;
  }

  m_SdlWindow = SDL_CreateWindow(title.data(), static_cast<int>(width), static_cast<int>(height), 0);
  if (m_SdlWindow == nullptr)
  {
    LOG_CORE_ERROR("failed creating window: {}", SDL_GetError());
    FOUR_ASSERT(false);
    DestroyWindow();
  }
  LOG_CORE_INFO("Window in sdlWindow Created.");
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
  EventType eventType = TransformEvent(event);

  // handle close event
  if (eventType == EventType::WindowClose)
  {
    OnCloseEvent();
    return;
  }

  if (m_EventList.contains(eventType))
  {
    if (auto* value = std::get_if<Event<WindowResizeEvent, uint32_t, uint32_t>>(&m_EventList[eventType]))
    {
      value->Notify(GetWidth(), GetHeight());
      return;
    }
  }
}

bool SdlWindow::ShouldClose() const noexcept
{
  return m_ShouldCose;
}

void SdlWindow::OnCloseEvent()
{
  m_ShouldCose = true;
}

EventType SdlWindow::TransformEvent(const SDL_Event& event)
{
  switch (event.type)
  {
    case SDL_EventType::SDL_EVENT_QUIT:
      return EventType::WindowClose;
    case SDL_EventType::SDL_EVENT_WINDOW_RESIZED:
      return EventType::WindowResize;
  }

  return {};
}

} // namespace four
