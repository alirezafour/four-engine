#include <iostream>
#include <format>

int main(int argc, char** argv)
{
    std::cout << std::format("Hello {}\n", "world");
    return 0;
}
