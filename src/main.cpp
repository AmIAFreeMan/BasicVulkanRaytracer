#include "VulkanApp.h"

/**
 * @brief Main entry point for the Vulkan Compute Raytracer application.
 * Instantiates the VulkanApp object, catches any standard runtime exceptions, and reports fatal errors.
 */
int main(int argc, char* argv[]) {
    try {
        VulkanApp app;
        std::cout << "Application running with a modular multi-file structure!" << std::endl;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}