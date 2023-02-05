#include "vulkan/engine.h"
#include "SDL2/SDL.h"


using namespace ae::gui;


Window::Window(const string& title, const uint16_t width, const uint16_t height) : _width(width), _height(height), _title(title)
{
}

void Window::init() {
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
        throw std::exception("SDL Video is not initialized");

    _window = SDL_CreateWindow(_title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        _width, _height,
        SDL_WINDOW_VULKAN);

    if (!_window) {
        throw std::runtime_error("SDL Window could not be initialized: " + string(SDL_GetError()));
    }
}
