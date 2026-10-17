#include "VulkanApp.h"

/**
 * @brief Finds a suitable physical device memory type matching the required filter and properties.
 */
static uint32_t findMemoryType(VkPhysicalDevice physicalDev, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDev, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type!");
}

/**
 * @brief Creates and populates the GPU storage buffer containing scene objects and camera data.
 */
void VulkanApp::createSceneBuffer() {
    struct GPUObject {
        int type;
        float _pad0[3];
        Vector3 position;
        float _pad1;
        Vector3 color;
        float _pad2;
        float scale;
        int materialType;
        float roughness;
        float _pad3;
    };

    struct GPUSceneData {
        GPUObject objects[10];
        int objectCount;
        float cameraYaw;
        float cameraPitch;
        float _padMeta;
        Vector3 cameraPos;
        float _padPos;
    } gpuScene{};

    gpuScene.objectCount = static_cast<int>(sceneManager.objects.size());
    for (size_t i = 0; i < sceneManager.objects.size() && i < 10; i++) {
        gpuScene.objects[i].type = sceneManager.objects[i].type;
        gpuScene.objects[i].position = sceneManager.objects[i].position;
        gpuScene.objects[i].color = sceneManager.objects[i].color;
        gpuScene.objects[i].scale = sceneManager.objects[i].scale;
        gpuScene.objects[i].materialType = sceneManager.objects[i].materialType;
        gpuScene.objects[i].roughness = sceneManager.objects[i].roughness;
    }
    gpuScene.cameraYaw = cameraYaw;
    gpuScene.cameraPitch = cameraPitch;
    gpuScene.cameraPos = cameraPos;

    VkDeviceSize bufferSize = sizeof(GPUSceneData);

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &sceneBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create scene storage buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, sceneBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &sceneBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate scene buffer memory!");
    }

    vkBindBufferMemory(device, sceneBuffer, sceneBufferMemory, 0);

    void* data;
    vkMapMemory(device, sceneBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, &gpuScene, (size_t)bufferSize);
    vkUnmapMemory(device, sceneBufferMemory);
}

/**
 * @brief Updates the GPU scene buffer memory map with latest camera and scene object changes.
 */
void VulkanApp::updateSceneBuffer() {
    if (sceneBufferMemory == VK_NULL_HANDLE) return;

    struct GPUObject {
        int type;
        float _pad0[3];
        Vector3 position;
        float _pad1;
        Vector3 color;
        float _pad2;
        float scale;
        int materialType;
        float roughness;
        float _pad3;
    };

    struct GPUSceneData {
        GPUObject objects[10];
        int objectCount;
        float cameraYaw;
        float cameraPitch;
        float _padMeta;
        Vector3 cameraPos;
        float _padPos;
    } gpuScene{};

    gpuScene.objectCount = static_cast<int>(sceneManager.objects.size());
    for (size_t i = 0; i < sceneManager.objects.size() && i < 10; i++) {
        gpuScene.objects[i].type = sceneManager.objects[i].type;
        gpuScene.objects[i].position = sceneManager.objects[i].position;
        gpuScene.objects[i].color = sceneManager.objects[i].color;
        gpuScene.objects[i].scale = sceneManager.objects[i].scale;
        gpuScene.objects[i].materialType = sceneManager.objects[i].materialType;
        gpuScene.objects[i].roughness = sceneManager.objects[i].roughness;
    }
    gpuScene.cameraYaw = cameraYaw;
    gpuScene.cameraPitch = cameraPitch;
    gpuScene.cameraPos = cameraPos;

    void* data;
    vkMapMemory(device, sceneBufferMemory, 0, sizeof(GPUSceneData), 0, &data);
    memcpy(data, &gpuScene, sizeof(GPUSceneData));
    vkUnmapMemory(device, sceneBufferMemory);
}