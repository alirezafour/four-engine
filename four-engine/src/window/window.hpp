#pragma once

#include <concepts>
#include <exception>
#include <memory>
#include <cstdint>
#include <string_view>

namespace four
{

/**
  * @brinef iterface / builder class for window
  * this class is base class window and it also act as builder for window creation
  */
template <typename Derived>
class Window
{
public:
  /**
    * @brief Create window of the requested type
    * Create Window of the type it can throw exception if fail
    * @param title the title for the window
    * @param width window width
    * @param height window height
    */
  static std::unique_ptr<Derived> CreateWindow(std::string_view title, int32_t width, int32_t height)
  {
    static_assert(std::derived_from<Derived, Window<Derived>>, "your window class is not derived form Window<>.");
    static_assert(std::constructible_from<Derived, std::string_view, int32_t, int32_t>,
                  "Constrcutor with (std::string_view, int32_t, int32_t) parameter is not exist in derived class.");

    try
    {
      auto window = std::make_unique<Derived>(title, width, height);
      return std::move(window);
    } catch (std::exception& e)
    {
      throw e;
    }
    return nullptr;
  }

private:
  friend Derived;
};

} // namespace four
