#pragma once

#include <SDL2/SDL.h> 
#include <string>

namespace ue {
    class Window {
        public:
            Window(int width, int height, const std::string& title);
            ~Window();

            Window(const Window&) = delete;
            Window& operator=(const Window&) = delete;

            bool pollEvents();
            SDL_Window* handle() const;
        private:
            SDL_Window* window = nullptr;
    };
}
