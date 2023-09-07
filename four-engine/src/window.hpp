#ifndef FOUR_WINDOW_HPP
#define FOUR_WINDOW_HPP

#include <memory>
#include <string>

#include <cassert>
#include <concepts>
#include <iostream>
#include <string_view>

namespace four::Engine2D
{

template <typename Derived>
class Window
{
public:
    /** return instance of the sub class */
    static Derived* GetInstance()
    {
        static_assert(std::derived_from<Derived, Window<Derived>>, "not base of window");
        return Derived::GetInstance();
    }

    static Derived* MakeWindow(std::string_view title, int32_t height, int32_t width)
    {
        static_assert(std::derived_from<Derived, Window<Derived>>, "not base of window");

        std::cout << "calling derived to give me instance\n";
        return Derived::MakeWindow(title, height, width);
    }

private:
    Window()
    {
    }
    Window(Window&)                        = delete;
    Window(Window&&)                       = delete;
    Window operator=(const Window&) const  = delete;
    Window operator=(const Window&&) const = delete;

private:
    friend Derived;
};

} // namespace four::Engine2D

#endif // !FOUR_WINDOW_HPP

// derived class include should be after Window class
#include "sdlWindow.hpp"
