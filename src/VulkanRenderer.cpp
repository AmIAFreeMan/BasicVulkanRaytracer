#include "VulkanApp.h"

/**
 * @brief Creates the command pool for command buffer allocation.
 */
void VulkanApp::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.computeAndGraphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

/**
 * @brief Allocates and records command buffers for compute dispatches and image layout barriers.
 */
void VulkanApp::createCommandBuffers() {
    commandBuffers.resize(swapChainImages.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }

    for (size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

        // Transition image layout to GENERAL for compute writes
        VkImageMemoryBarrier barrierToGeneral{};
        barrierToGeneral.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrierToGeneral.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrierToGeneral.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrierToGeneral.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierToGeneral.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierToGeneral.image = swapChainImages[i];
        barrierToGeneral.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrierToGeneral.subresourceRange.baseMipLevel = 0;
        barrierToGeneral.subresourceRange.levelCount = 1;
        barrierToGeneral.subresourceRange.baseArrayLayer = 0;
        barrierToGeneral.subresourceRange.layerCount = 1;
        barrierToGeneral.srcAccessMask = 0;
        barrierToGeneral.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

        vkCmdPipelineBarrier(commandBuffers[i], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
            0, 0, nullptr, 0, nullptr, 1, &barrierToGeneral);

        // Dispatch compute raytracer work groups
        vkCmdDispatch(commandBuffers[i], (swapChainExtent.width + 15) / 16, (swapChainExtent.height + 15) / 16, 1);

        // Transition image layout to PRESENT_SRC for display
        VkImageMemoryBarrier barrierToPresent{};
        barrierToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrierToPresent.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrierToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrierToPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierToPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierToPresent.image = swapChainImages[i];
        barrierToPresent.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrierToPresent.subresourceRange.baseMipLevel = 0;
        barrierToPresent.subresourceRange.levelCount = 1;
        barrierToPresent.subresourceRange.baseArrayLayer = 0;
        barrierToPresent.subresourceRange.layerCount = 1;
        barrierToPresent.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrierToPresent.dstAccessMask = 0;

        vkCmdPipelineBarrier(commandBuffers[i], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
            0, 0, nullptr, 0, nullptr, 1, &barrierToPresent);

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }
}

/**
 * @brief Creates synchronization objects (semaphores and fences).
 */
void VulkanApp::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create synchronization objects!");
    }
}

/**
 * @brief Submits command buffers and presents the rendered frame to the screen.
 */
void VulkanApp::drawFrame() {
    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inFlightFence);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(computeQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit compute command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(presentQueue, &presentInfo);
}