#include "VulkanApp.h"
#include <cmath>

/**
 * @brief Constructor: Sets up the default scene, initializes SDL, window, Vulkan, and sync structures.
 */
VulkanApp::VulkanApp() {
    sceneManager.loadDefaultScene();
    
    initSDL();
    initWindow();
    initVulkan();
    createSyncObjects();
}

/**
 * @brief Destructor: Cleans up all allocated Vulkan and SDL resources.
 */
VulkanApp::~VulkanApp() {
    cleanup();
}

/**
 * @brief Main application execution loop handling event polling, camera updates, and frame rendering.
 */
void VulkanApp::run() {
    bool isRunning = true;
    SDL_Event event;

    std::cout << "[App] Controls Set: W (Forward), S (Backward), A (Left), D (Right), ArrowUp (Up), ArrowDown (Down), Mouse Drag (Look)." << std::endl;

    while (isRunning) {
        // 1. Process Window and Input Events (SDL Event Polling)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
            }
            // Handle Drag-and-Drop 3D Model files directly into the window
            else if (event.type == SDL_DROPFILE) {
                char* droppedFileDir = event.drop.file;
                SceneObject customObj{};
                customObj.type = static_cast<int>(ObjectType::CUSTOM_MESH);
                customObj.position = {0.0f, 0.0f, 4.0f};
                customObj.color = {1.0f, 0.84f, 0.0f};
                customObj.scale = 1.2f;
            
                customObj.materialType = 0; // Default to matte
                customObj.roughness = 0.0f;

                sceneManager.objects.push_back(customObj);
                std::cout << "[Scene] File dropped: " << droppedFileDir << std::endl;
                SDL_free(droppedFileDir);
                updateSceneBuffer();
            }
            // Mouse Click & Drag Rotation Handling
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    isDragging = true;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    isDragging = false;
                }
            }
            else if (event.type == SDL_MOUSEMOTION && isDragging) {
                cameraYaw -= event.motion.xrel * 0.005f;
                cameraPitch -= event.motion.yrel * 0.005f;
                cameraPitch = std::clamp(cameraPitch, -1.5f, 1.5f); // Clamp pitch to prevent flipping
            }
        }

        // 2. Continuous Keyboard Polling for Free-Flight Camera Movement (WASD + Arrows)
        const Uint8* keystate = SDL_GetKeyboardState(nullptr);
        float moveSpeed = 0.08f;
       
        if (keystate[SDL_SCANCODE_W]) { cameraPos.z += moveSpeed; } // Forward
        if (keystate[SDL_SCANCODE_S]) { cameraPos.z -= moveSpeed; } // Backward
        if (keystate[SDL_SCANCODE_A]) { cameraPos.x -= moveSpeed; } // Strafe Left
        if (keystate[SDL_SCANCODE_D]) { cameraPos.x += moveSpeed; } // Strafe Right
        if (keystate[SDL_SCANCODE_UP]) { cameraPos.y += moveSpeed; }   // Elevate Up
        if (keystate[SDL_SCANCODE_DOWN]) { cameraPos.y -= moveSpeed; } // Descend Down
        
        // 3. Update GPU Uniform/Storage Buffers with new camera and scene states
        updateSceneBuffer();
        
        // 4. Render the current frame via Vulkan Compute Raytracing
        drawFrame();
    }

    // Ensure the GPU has finished all tasks before exiting
    if (device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device);
    }
}

/**
 * @brief Initializes the SDL video subsystem.
 */
void VulkanApp::initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
    }
}

/**
 * @brief Creates the primary application window with Vulkan support flags.
 */
void VulkanApp::initWindow() {
    window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN
    );

    if (!window) {
        throw std::runtime_error("Failed to create SDL window: " + std::string(SDL_GetError()));
    }
}

/**
 * @brief Safely releases all Vulkan objects and destroys SDL windows in reverse order of creation.
 */
void VulkanApp::cleanup() {
    if (device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device);

        if (imageAvailableSemaphore != VK_NULL_HANDLE) vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
        if (renderFinishedSemaphore != VK_NULL_HANDLE) vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
        if (inFlightFence != VK_NULL_HANDLE) vkDestroyFence(device, inFlightFence, nullptr);

        if (swapChain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device, swapChain, nullptr);
            swapChain = VK_NULL_HANDLE;
        }

        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
    }

    if (surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }

    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    SDL_Quit();
}