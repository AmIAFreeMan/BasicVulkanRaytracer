==================================================
VULKAN COMPUTE RAYTRACER - PROJECT DOCUMENTATION
==================================================

1. OVERVIEW
-----------
This application is a high-performance, real-time 3D raytracer built from scratch 
using modern C++17 and the Vulkan API on Fedora. It leverages GPU-accelerated compute 
shaders (SPIR-V) to perform ray-primitive intersections per pixel, streaming direct output 
to the Vulkan swapchain via storage images. 

Key Features:
- GPU-Side Raytracing: Real-time fragment evaluation inside `raytrace.comp`.
- Modular Architecture: Clean separation of windowing, Vulkan core logic, scene management, and input.
- Interactive 3D Camera: Mouse click-and-drag rotation (Yaw/Pitch) and keyboard movement (WASD/Arrows).


2. PROJECT STRUCTURE & FILE BREAKDOWN
-------------------------------------
The project follows a clean, modular multi-file structure for maintainability and scalability:

~/include/
    ├── VulkanApp.h       - Main class definition, Vulkan handle declarations, camera/input states, 
    │                       and pipeline configuration methods.
    └── Scene.h           - Defines geometric primitives (Planes, Spheres, Cones, Squares), 
                            object types, and the `SceneManager` default scene generator.

~/src/
    ├── main.cpp          - Entry point (`main()`) responsible for instantiating `VulkanApp` 
                            and handling top-level application exceptions.
    ├── App.cpp           - SDL2 event loop implementation, window creation, cleanup routines, 
                            mouse motion/drag handling, drag-and-drop event processing, and real-time 
                            keyboard polling.
    └── VulkanCore.cpp    - Low-level Vulkan setup: Instance creation, physical device selection, 
                            logical device & queue family management, swapchain setup, image views, 
                            compute pipeline creation, descriptor pools/sets, command buffers, synchronization 
                            fences/semaphores, and persistent Shader Storage Buffer Object (SSBO) updates.

~/shaders/
    ├── raytrace.comp     - GLSL compute shader (local workgroup size 16x16) that generates camera rays 
                            based on yaw/pitch orientation and evaluates intersections against planes and spheres 
                            stored in the GPU SSBO buffer.
    └── raytrace.comp.spv - Compiled SPIR-V binary version of the compute shader loaded at runtime.


3. CONTROLS
-----------
- Look Around: Left-Click & Drag mouse.
- Move Forward/Backward: W / S keys.
- Move Left/Right (Strafe): A / D keys.
- Move Up/Down: Up Arrow / Down Arrow keys.
- Add Object: Drag and drop any 3D file directly onto the window.
- Exit: ESC key or window close button.



--------------------------------------------------------------------------------------------

## Mathematical Framework

The raytracer relies on analytic geometry, linear algebra, and trigonometry to render 3D scenes on the GPU.

### 1. Screen-to-World Ray Generation
* **Screen Pixel Normalization**: Maps 2D screen pixel coordinates into normalized device coordinates ranging from -1.0 to 1.0.
* **Ray Equation**: Defines a ray by its starting point (origin) and a trajectory vector scaled by distance.

### 2. Camera Rotation
* **Yaw Rotation Matrix**: Rotates the camera view horizontally around the vertical Y-axis based on mouse input.
* **Pitch Rotation Matrix**: Rotates the camera view vertically around the horizontal X-axis.

### 3. Object Intersection Formulas
* **Infinite Plane Intersection**: Calculates the exact point where a ray strikes a flat, horizontal floor at a fixed height.
* **Sphere Intersection**: Uses the quadratic formula (discriminant method) to find where a ray enters and exits a spherical volume.
* **Bounded Cone Intersection**: Solves quadratic coefficients combined with height-clipping bounds to render a single, finite cone pointing upward.
* **Cube / Box Intersection (Slab Method)**: Treats a cube as three pairs of parallel flat planes, testing ray entry and exit times across each axis.