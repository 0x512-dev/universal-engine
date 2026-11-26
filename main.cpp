#include "src/platform/Window.h"
#include "src/gpu/GpuContext.h"

#include <iostream>

int main() {
    try {
        ue::Window window(720, 1650, "test");
        ue::GpuContext gpu(window.handle());
        
        std::cout << "Vulkan instance OK\n";
        
        bool running = true;

        while (running) {
            running = window.pollEvents();
            SDL_Delay(10);
        }
    } catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
