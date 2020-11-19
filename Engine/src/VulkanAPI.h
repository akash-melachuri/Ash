#pragma once

#include <vulkan/vulkan.hpp>

namespace Ash {

class VulkanAPI {
   public:
    VulkanAPI();
    ~VulkanAPI();

    void init();
    void cleanup();

   private:
    bool checkValidationSupport();
    void createInstance();
    void setupDebugMessenger();

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};

}  // namespace Ash
