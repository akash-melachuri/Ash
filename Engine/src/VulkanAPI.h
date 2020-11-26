#pragma once

#include <vulkan/vulkan.hpp>

namespace Ash {

class VulkanAPI {
   public:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
    };

   public:
    VulkanAPI();
    ~VulkanAPI();

    void init();
    void cleanup();

   private:
    bool checkValidationSupport();
    void createInstance();
    void setupDebugMessenger();
    void pickPhysicalDevice();
    void createLogicalDevice();

    VkInstance instance;

    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

#ifndef ASH_DEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};

}  // namespace Ash
