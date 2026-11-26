#include "gpu/GpuContext.h"

#include <SDL2/SDL_vulkan.h>

#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <set>

namespace ue {
    GpuContext::GpuContext(SDL_Window* window) {
        createInstance(window);
        createSurface(window);
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapchain(window);
    }

    GpuContext::~GpuContext() {
        if (swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device, swapchain, nullptr);
        }

        if (device != VK_NULL_HANDLE) {
            vkDestroyDevice(device, nullptr);
        }

        if (surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance, surface, nullptr);
        }

        if (instance != VK_NULL_HANDLE) {
            vkDestroyInstance(instance, nullptr);
        }
    }

    void GpuContext::createInstance(SDL_Window* window) {
        unsigned int extensionCount = 0;
        
        if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr)) {
            throw std::runtime_error(std::string("SDL extensions: ") + SDL_GetError());
        }
        
        std::vector<const char*> extensions(extensionCount);
        
        if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data())) {
            throw std::runtime_error(std::string("SDL extensions: ") + SDL_GetError());
        }

        VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        
        appInfo.pApplicationName = "Universal Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        appInfo.pEngineName = "Universal Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.pApplicationInfo = &appInfo;

        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        
        if (result != VK_SUCCESS) {
            throw std::runtime_error("vkCreateInstance failed, code " + std::to_string(static_cast<int>(result)));
        }
    }

    void GpuContext::createSurface(SDL_Window* window) {
        if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
            throw std::runtime_error(std::string("SDL_Vulkan_CreateSurface: ") + SDL_GetError());
        }
    }

    GpuContext::QueueFamilies GpuContext::findQueueFamilies(VkPhysicalDevice candidate) const {
        QueueFamilies result;

        uint32_t count = 0;
        
        vkGetPhysicalDeviceQueueFamilyProperties(candidate, &count, nullptr);
        
        std::vector<VkQueueFamilyProperties> families(count);
   
        vkGetPhysicalDeviceQueueFamilyProperties(candidate, &count, families.data());
        
        for (uint32_t i = 0; i < count; ++i) {
            if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                result.graphics = i;
            }

            VkBool32 presentSupport = VK_FALSE;
            
            vkGetPhysicalDeviceSurfaceSupportKHR(candidate, i, surface, &presentSupport);
            
            if (presentSupport) {
                result.present = i;
            }

            if (result.complete()) break;
        }
        return result;
    }
    
    bool GpuContext::hasSwapchainExtension(VkPhysicalDevice candidate) const {
        uint32_t count = 0;
        
        vkEnumerateDeviceExtensionProperties(candidate, nullptr, &count, nullptr);
        
        std::vector<VkExtensionProperties> available(count);
        
        vkEnumerateDeviceExtensionProperties(candidate, nullptr, &count, available.data());

        for (const VkExtensionProperties& ext : available) {
            if (std::strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
                return true;
            }
        }
        return false;
    }

    bool GpuContext::isSuitable(VkPhysicalDevice candidate) const {
        return findQueueFamilies(candidate).complete() && hasSwapchainExtension(candidate);
    }

    void GpuContext::pickPhysicalDevice() {
        uint32_t count = 0;
        
        vkEnumeratePhysicalDevices(instance, &count, nullptr);
        
        if (count == 0) {
            throw std::runtime_error("No Vulkan GPU found");
        }
        
        std::vector<VkPhysicalDevice> devices(count);
        
        vkEnumeratePhysicalDevices(instance, &count, devices.data());

        for (VkPhysicalDevice candidate : devices) {
            if (isSuitable(candidate)) {
                physicalDevice = candidate;
                break;
            }
        }
        
        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("No suitable GPU (graphics + present + swapchain)");
        }
        
        queueFamilies = findQueueFamilies(physicalDevice);
        
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(physicalDevice, &props);
        
        std::cout << "Vulkan GPU: " << props.deviceName << "\n";
    }

    void GpuContext::createLogicalDevice() {
        std::set<uint32_t> uniqueFamilies = {*queueFamilies.graphics, *queueFamilies.present};

        const float priority = 1.0f;

        std::vector<VkDeviceQueueCreateInfo> queueInfos;

        for (uint32_t family : uniqueFamilies) {
            VkDeviceQueueCreateInfo queueInfo{};

            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = family;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &priority;
            queueInfos.push_back(queueInfo);
        }

        const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkPhysicalDeviceFeatures features{};

        VkDeviceCreateInfo createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
        createInfo.pQueueCreateInfos = queueInfos.data();
        createInfo.pEnabledFeatures = &features;
        createInfo.enabledExtensionCount = 1;
        createInfo.ppEnabledExtensionNames = deviceExtensions;

        VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("vkCreatDevice failed, code: " + std::to_string(static_cast<int>(result)));
        }

        vkGetDeviceQueue(device, *queueFamilies.graphics, 0, &graphicsQueue);
        vkGetDeviceQueue(device, *queueFamilies.present, 0, &presentQueue);

        std::cout << "Vulkan device OK\n";
    }

    void GpuContext::createSwapchain(SDL_Window* window) {
        VkSurfaceCapabilitiesKHR capabilities{};

        VkResult capsResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

        if (capsResult != VK_SUCCESS) {
            throw std::runtime_error("vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed, code: " + std::to_string(static_cast<int>(capsResult)));
        }

        if (!(capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
            throw std::runtime_error("Swapchain images do not support TRANSFER_DST");
        }

        uint32_t formatCount = 0;

        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

        if (formatCount == 0) {
            throw std::runtime_error("No surface formats");
        }

        std::vector<VkSurfaceFormatKHR> formats(formatCount);

        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

        VkSurfaceFormatKHR chosen = formats[0];

        for (const VkSurfaceFormatKHR format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                chosen = format;
                break;
            }
        }

        swapchainFormat = chosen.format;

        if (capabilities.currentExtent.width != UINT32_MAX) {
            swapchainExtent = capabilities.currentExtent;
        } else {
            int width = 0;
            int height = 0;

            SDL_Vulkan_GetDrawableSize(window, &width, &height);

            swapchainExtent.width = std::clamp(static_cast<uint32_t>(std::max(width, 1)), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            swapchainExtent.height = std::clamp(static_cast<uint32_t>(std::max(height, 1)), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }

        uint32_t imageCount = capabilities.minImageCount + 1;
        
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }

        uint32_t familyIndices[] = {*queueFamilies.graphics, *queueFamilies.present};
        
        VkSwapchainCreateInfoKHR createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = chosen.format;
        createInfo.imageColorSpace = chosen.colorSpace;
        createInfo.imageExtent = swapchainExtent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        if (*queueFamilies.graphics != *queueFamilies.present) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = familyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        createInfo.clipped = VK_TRUE;

        VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("vkCreateSwapchainKHR failed, code: " + std::to_string(static_cast<int>(result)));
        }

        uint32_t actualCount = 0;

        vkGetSwapchainImagesKHR(device, swapchain, &actualCount, nullptr);

        swapchainImages.resize(actualCount);

        vkGetSwapchainImagesKHR(device, swapchain, &actualCount, swapchainImages.data());

        std::cout << "Swapchain OK: " << swapchainExtent.width << "x" << swapchainExtent.height << ", images: " << actualCount << "\n";
    }
}