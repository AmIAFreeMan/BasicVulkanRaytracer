#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <algorithm>

// Application constants for window configuration[cite: 10]
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const char* const WINDOW_TITLE = "Vulkan Compute Raytracer";

/**
 * @brief Helper structure to manage Vulkan Queue Family indices (compute/graphics and presentation)[cite: 10].
 */
struct QueueFamilyIndices {
    std::optional<uint32_t> computeAndGraphicsFamily;
    std::optional<uint32_t> presentFamily;

    /// Verifies if required queue families are found and complete[cite: 10].
    bool isComplete() const {
        return computeAndGraphicsFamily.has_value() && presentFamily.has_value();
    }
};

/**
 * @brief Core Application Class declaration defining basic Vulkan handles, queue queues, 
 * frame synchronization primitives, and setup routines[cite: 10].
 */
class VulkanApp {
public:
    VulkanApp();
    ~VulkanApp();

    void run();

private:
    // Window and Core Vulkan Handles[cite: 10]
    SDL_Window* window = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    // Frame Synchronization Primitives[cite: 10]
    VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence inFlightFence = VK_NULL_HANDLE;

    // Core Lifecycle & Initialization Methods[cite: 10]
    void initSDL();
    void initWindow();
    void initVulkan();
    void createInstance();
    void createSurface();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice deviceToQuery);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createSyncObjects();

    // Frame Loop & Cleanup Methods[cite: 10]
    void drawFrame();
    void cleanup();
};