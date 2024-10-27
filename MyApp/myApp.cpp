#include "myApp.hpp"
#include "core/engine.hpp"

MyApp::MyApp(const std::string& title, uint32_t width, uint32_t height) : four::Application{title, width, height}
{
  GetEngine()->AddApplicaiton(this);
}

//==================================================================================================
void MyApp::Init()
{
}

//==================================================================================================
void MyApp::OnUpdate(float deltaTime)
{
}
