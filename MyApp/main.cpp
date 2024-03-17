#include "myApp.hpp"


constexpr uint64_t WIDTH  = 1024;
constexpr uint64_t HEIGHT = 768;

int main()
{
  MyApp app("MyApp", WIDTH, HEIGHT);
  app.Run();
  return EXIT_SUCCESS;
}
