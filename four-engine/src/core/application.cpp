#include "four-pch.hpp"

#include "core/application.hpp"
#include "core/engine.hpp"

namespace four
{
Application::Application(std::string_view title, std::uint32_t width, std::uint32_t height) :
m_Engine(Engine::Init(title, width, height))
{
  m_Engine->AddImGuiLayer(std::make_unique<ImGuiLayer>());
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
