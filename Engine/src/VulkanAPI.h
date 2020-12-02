#pragma once

// Make sure that vulkan header is included before GLFW
#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

#include "Core.h"

namespace Ash {

class VulkanAPI {
   public:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentsFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentsFamily.has_value();
        }
    };

   public:
    VulkanAPI();
    ~VulkanAPI();

    void init(GLFWwindow* window);
    void cleanup();

   private:
    bool checkValidationSupport();
    void createInstance();
    void setupDebugMessenger();
    void createSurface(GLFWwindow* window);
    void pickPhysicalDevice();
    void createLogicalDevice();

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    VkInstance instance;

    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSurfaceKHR surface;

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifndef ASH_DEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};

}  // namespace Ash
