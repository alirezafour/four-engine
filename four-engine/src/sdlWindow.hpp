#ifndef FOUR_SDLWINDOW_HPP
#define FOUR_SDLWINDOW_HPP

#include "window.hpp"

namespace four::Engine2D
{

class sdlWindow : public Window<sdlWindow>
{
public:
    static sdlWindow* GetInstance()
    {
        if (m_Window)
        {
            std::cout << "return already made instance of sdlWindow\n";
            return m_Window;
        }
        std::cout << "first time creating instance of sdlWindow\n";
        m_Window = new sdlWindow();
        return m_Window;
    }
    inline static sdlWindow* MakeWindow(std::string_view title, int32_t height, int32_t width)
    {
        std::cout << "get instance form sdlWindow\n";
        return GetInstance();
    }
    ~sdlWindow()
    {
        delete m_Window;
    }

private:
    sdlWindow() : Window<sdlWindow>()
    {
    }
    sdlWindow(sdlWindow&)                        = delete;
    sdlWindow(sdlWindow&&)                       = delete;
    sdlWindow operator=(const sdlWindow&) const  = delete;
    sdlWindow operator=(const sdlWindow&&) const = delete;

private:
    inline static sdlWindow* m_Window = nullptr;
};

} // namespace four::Engine2D
#endif // !FOUR_SDLWINDOW_HPP
