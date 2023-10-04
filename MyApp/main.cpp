#include "core/engine.hpp"

int main()
{
  static constexpr uint32_t WIDTH  = 800;
  static constexpr uint32_t HEIGHT = 600;

  auto* app = four::Engine::Init("title", WIDTH, HEIGHT);
  app->Run();
  app->Shutdown();
  return 0;
}
