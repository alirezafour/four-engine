#include <iostream>
#include <format>
#include "SDL.h"

int main(int argc, char** argv)
{
    std::cout << std::format("Hello {}\n", "world");
    return 0;
}
