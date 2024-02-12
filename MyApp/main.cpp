#include "GLFW/glfw3.h"
#include "core/engine.hpp"

constexpr int64_t WIDTH  = 1024;
constexpr int64_t HEIGHT = 768;

int main()
{
  auto* app = four::Engine::Init("title", WIDTH, HEIGHT);
  app->Run();
  app->Shutdown();
  return EXIT_SUCCESS;
}
