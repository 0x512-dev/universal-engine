#include "platform/Window.h"

#include <stdexcept>
#include <iostream>

namespace ue {
    Window::Window(int width, int height, const std::string& title) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw std::runtime_error(std::string("SDL_Init: ") + SDL_GetError());
        }
        
        window = SDL_CreateWindow(
            title.c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
        );
        
        if (window == nullptr) {
            SDL_Quit();
            throw std::runtime_error(std::string("SDL_CreateWindow: ") + SDL_GetError());
        }
    }

    Window::~Window() {
        if (window != nullptr) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
    }

    bool Window::pollEvents() {
        SDL_Event event;
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) 
                return false;
            
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) 
                return false;
        }
        return true;
    }

    SDL_Window* Window::handle() const {
        return window;
    }
}
