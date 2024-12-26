#include "myApp.hpp"
#include "core/engine.hpp"

MyApp::MyApp(std::string_view title, std::uint32_t width, std::uint32_t height) :
four::Application{title, width, height}
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
