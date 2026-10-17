cmake_minimum_required(VERSION 3.10)
project(Raytracer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find Vulkan
find_package(Vulkan REQUIRED)

# 2. Find SDL2 safely via PkgConfig
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

# 3. Define your executable with all modular source files
add_executable(Raytracer
    src/main.cpp
    src/App.cpp
    src/VulkanContext.cpp
    src/VulkanPipeline.cpp
    src/VulkanSwapchain.cpp
    src/VulkanBuffer.cpp
    src/VulkanRenderer.cpp
    include/VulkanApp.h
    include/Scene.h
)

# 4. Specify include directories
target_include_directories(Raytracer PRIVATE
    include
    ${Vulkan_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)

# 5. Link required external libraries
target_link_libraries(Raytracer PRIVATE 
    Vulkan::Vulkan
    ${SDL2_LIBRARIES}
)

if(SDL2_LIBRARY_DIRS)
    target_link_directories(Raytracer PRIVATE ${SDL2_LIBRARY_DIRS})
endif()