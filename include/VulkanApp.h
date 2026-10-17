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
#include <fstream>

#include "Scene.h"

// Application window dimensions and window title configuration constants
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const char* const WINDOW_TITLE = "Vulkan Compute Raytracer";

/**
 * @brief Helper structure used to query and verify compatible Vulkan Queue Families.
 */
struct QueueFamilyIndices {
    std::optional<uint32_t> computeAndGraphicsFamily;
    std::optional<uint32_t> presentFamily;

    /// Checks if both compute/graphics and presentation queue families are successfully available.
    bool isComplete() const {
        return computeAndGraphicsFamily.has_value() && presentFamily.has_value();
    }
};

/**
 * @brief Main Application class managing SDL window creation, Vulkan compute raytracing pipelines, 
 * descriptor configurations, scene data buffers, and camera/mouse interaction states.
 */
class VulkanApp {
public:
    VulkanApp();
    ~VulkanApp();

    void run();
    void updateSceneBuffer();

    // Camera and User Interaction State (tracking position, pitch, yaw, and drag status)
    Vector3 cameraPos = {0.0f, 2.0f, -4.0f}; // <-- Elevated to Y = 2.0 and pulled back to Z = -4.0
    float cameraYaw = 0.0f;
    float cameraPitch = 0.0f;
    bool isDragging = false;

private:
    // Window and Core Vulkan Handles
    SDL_Window* window = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    // Compute Pipeline and Descriptor Handles
    VkPipelineLayout computePipelineLayout = VK_NULL_HANDLE;
    VkPipeline computePipeline = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    // Command Pool and Command Buffers
    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;

    // GPU Storage Buffer for Scene Data and Objects
    VkBuffer sceneBuffer = VK_NULL_HANDLE;
    VkDeviceMemory sceneBufferMemory = VK_NULL_HANDLE;

    // Frame Synchronization Primitives
    VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence inFlightFence = VK_NULL_HANDLE;

    // Scene Manager handling 3D primitives and meshes
    SceneManager sceneManager;

    // Core Lifecycle, Initialization, and Setup Methods
    void initSDL();
    void initWindow();
    void initVulkan();
    void createInstance();
    void createSurface();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice deviceToQuery);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createSceneBuffer();
    void createDescriptorSetLayout();
    void createComputePipeline();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    // Frame Rendering and Cleanup Methods
    void drawFrame();
    void cleanup();
};