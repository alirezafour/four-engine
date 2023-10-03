#include "core/engine.hpp"

int main()
{
  auto* app = four::Engine::Init("title", 800, 600);
  app->Run();
  app->Shutdown();
  return 0;
}
