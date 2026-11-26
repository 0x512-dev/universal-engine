#pragma once

#include <SDL.h>
#include <vulkan/vulkan.h>

#include <vector>
#include <cstdint>
#include <optional>

namespace ue {
    class GpuContext {
        public:
            explicit GpuContext(SDL_Window* window);
            ~GpuContext();

            GpuContext(const GpuContext&) = delete;
            GpuContext& operator=(const GpuContext&) = delete;

        private:
            struct QueueFamilies {
                std::optional<uint32_t> graphics;
                std::optional<uint32_t> present;

                bool complete() const {
                    return graphics.has_value() && present.has_value();
                }
            };

            void createInstance(SDL_Window* window);
            void createSurface(SDL_Window* window);
            void createSwapchain(SDL_Window* window);
            void pickPhysicalDevice();
            void createLogicalDevice();
        
            QueueFamilies findQueueFamilies(VkPhysicalDevice candidate) const;
        
            bool hasSwapchainExtension(VkPhysicalDevice candidate) const;
            bool isSuitable(VkPhysicalDevice candidate) const;

            VkInstance instance = VK_NULL_HANDLE;
            VkSurfaceKHR surface = VK_NULL_HANDLE;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

            QueueFamilies queueFamilies;

            VkDevice device = VK_NULL_HANDLE;
            VkQueue graphicsQueue = VK_NULL_HANDLE;
            VkQueue presentQueue = VK_NULL_HANDLE;

            VkSwapchainKHR swapchain = VK_NULL_HANDLE;
            VkFormat swapchainFormat = VK_FORMAT_UNDEFINED;
            VkExtent2D swapchainExtent{};
            std::vector<VkImage> swapchainImages;
    };
}