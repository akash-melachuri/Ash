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
    VkInstance instance;
};

}  // namespace Ash
