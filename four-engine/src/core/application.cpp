#include "four-pch.h"

#include "core/application.hpp"
#include "core/engine.hpp"

namespace four
{
Application::Application(const std::string& title, uint32_t width, uint32_t height) :
m_Engine(Engine::Init(title, width, height))
{
}

Application::~Application()
{
  m_Engine->Shutdown();
}

void Application::Run()
{
  Init();
  m_Engine->Run();
}


} // namespace four
