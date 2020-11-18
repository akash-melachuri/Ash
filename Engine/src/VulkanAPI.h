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

    VkInstance instance;

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};

}  // namespace Ash
