#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

/// Simple 3D Vector structure used for positions, directions, and RGB colors.
struct Vector3 {
    float x, y, z;
};

/// Supported geometric object types rendered by the raytracing compute shader.
enum class ObjectType {
    PLANE = 0,
    SPHERE = 1,
    CONE = 2,
    SQUARE = 3,
    CUSTOM_MESH = 4
};

/// Represents a single 3D object instance within the scene (Strict std140 GPU alignment).
struct SceneObject {
    int type;            // Offset 0 (4 bytes)
    float _pad0[3];      // Offset 4 (12 bytes padding -> aligns position to 16)
    
    Vector3 position;    // Offset 16 (12 bytes)
    float _pad1;         // Offset 28 (4 bytes padding -> aligns color to 32)
    
    Vector3 color;       // Offset 32 (12 bytes)
    float _pad2;         // Offset 44 (4 bytes padding -> aligns scale to 48)
    
    float scale;         // Offset 48 (4 bytes)
    int materialType;    // Offset 52 (4 bytes)
    float roughness;     // Offset 56 (4 bytes)
    float _pad3;         // Offset 60 (4 bytes padding -> rounds total struct size to 64 bytes)
};

/// Manages the collection of scene objects and provides loading utilities.
class SceneManager {
public:
    std::vector<SceneObject> objects;

    /// Initializes a default scene containing foundational geometric primitives with unique materials.
    void loadDefaultScene() {
        objects.clear();

        // 1. Plane (Floor) - Matte Gray
        SceneObject plane{};
        plane.type = static_cast<int>(ObjectType::PLANE);
        plane.position = { 0.0f, 0.0f, 0.0f };
        plane.color = { 0.8f, 0.8f, 0.8f };
        plane.scale = 1.0f;
        plane.materialType = 0; // Matte
        plane.roughness = 0.0f;
        objects.push_back(plane);

        // 2. Sphere - Shiny Chrome Mirror
        SceneObject sphere{};
        sphere.type = static_cast<int>(ObjectType::SPHERE);
        sphere.position = { 0.0f, 1.0f, 5.0f };
        sphere.color = { 1.0f, 0.2f, 0.2f }; // Red tint
        sphere.scale = 1.0f;
        sphere.materialType = 1; // Shiny Metal
        sphere.roughness = 0.0f; // Mirror polish
        objects.push_back(sphere);

        // 3. Cone - Matte Blue
        SceneObject cone{};
        cone.type = static_cast<int>(ObjectType::CONE);
        cone.position = { -2.0f, 0.0f, 4.0f };
        cone.color = { 0.2f, 0.4f, 1.0f };
        cone.scale = 1.5f;
        cone.materialType = 0; // Matte
        objects.push_back(cone);

        // 4. Square / Box - Matte Green
        SceneObject cube{};
        cube.type = static_cast<int>(ObjectType::SQUARE);
        cube.position = { 2.0f, 1.0f, 6.0f };
        cube.color = { 0.2f, 1.0f, 0.2f };
        cube.scale = 1.0f;
        cube.materialType = 0; // Matte
        objects.push_back(cube);

        std::cout << "[Scene] Default scene loaded successfully with mixed materials!" << std::endl;
    }
};